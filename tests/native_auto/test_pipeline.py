import importlib.util
from pathlib import Path
import tempfile
import json
import unittest
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('pipeline',ROOT/'tools/native_auto/pipeline.py')
p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)
HASH='a'*64
class PipelineTests(unittest.TestCase):
    def sample(self):
        return {'schema':'chq-ghidra-static-findings-v1','cpu':'A','rawRomSha256':HASH,
          'romBase':'0x000000','romEndExclusive':'0x080000','sites':[{'pc':'0x00820C',
          'nearbyInstructions':[{'pc':'0x00820C'},{'pc':'0x00820E'}]}]}
    def contract(self):
        return {'routineId':'steering_demo','cpu':'A','romSha256':HASH,'inputs':['raw'],
          'outputs':['processed'],'registerEffects':[],'memoryEffects':[],'calls':[],
          'cyclePolicy':'not modeled','kind':'signed-i16-lookup','reviewed':True,
          'readyForGeneration':True,'unresolvedDependencies':[], 'inputMin':-1,'inputMax':1,
          'table':[-2,0,3],'fixtures':[{'raw':-1,'originalOutput':-2},{'raw':1,'originalOutput':3}]}
    def test_static_requires_provenance(self):
        self.assertEqual(len(p.source_report(self.sample())),1)
        x=self.sample();x['rawRomSha256']='bad'
        with self.assertRaises(ValueError):p.source_report(x)
    def test_denominator_not_invented(self):
        s=p.source_report(self.sample())
        result=p.build_ledger(s,[{'cpu':'A','romSha256':HASH,'pc':'0x00820C','hits':123}])
        self.assertEqual(result['routines'][0]['executedUniqueWithinStatic'],1)
        self.assertEqual(result['routines'][0]['dynamicExecutionsWithinStatic'],123)
        self.assertIsNone(result['completionPercent'])
        with self.assertRaises(ValueError):p.build_ledger(s,[{'cpu':'A','romSha256':HASH,'pc':'0x00820C','hits':1}]*2)
    def test_generate_fail_closed(self):
        c=self.contract();self.assertIn('std::array',p.generate_candidate(c))
        c['unresolvedDependencies']=['A112'];
        with self.assertRaises(ValueError):p.generate_candidate(c)
    def test_verifier_cannot_promote(self):
        c=self.contract();r=p.verify_cases(c,c['fixtures'])
        self.assertEqual(r['matched'],2);self.assertFalse(r['liveShadowVerified'])
        self.assertFalse(p.plan_promotion(c,r)['authorized'])
    def test_orchestration_and_reproducibility(self):
        with tempfile.TemporaryDirectory() as td:
            td=Path(td);(td/'gh.json').write_text(json.dumps(self.sample()))
            (td/'contracts.json').write_text(json.dumps([self.contract()]))
            manifest={'schema':p.SCHEMA,'ghidraReports':[str(td/'gh.json')],
                      'contracts':str(td/'contracts.json')}
            out=td/'out';a=p.orchestrate(manifest,out)
            self.assertEqual(a[0]['status'],'offline-pass')
            self.assertFalse(a[0]['promotionAuthorized'])
            before=(out/'queue.json').read_bytes();p.orchestrate(manifest,out)
            self.assertEqual(before,(out/'queue.json').read_bytes())
if __name__=='__main__':unittest.main()
