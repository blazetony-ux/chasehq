import unittest
from sprite_pipeline_correlate import correlate


class PipelineTest(unittest.TestCase):
    def test_known_run029_stage_order(self):
        p = {'schema': 'chq-sprite-pipeline-events-v1',
             'prepared': [{'frame': 9549, 'map': 19, 'x': 210}],
             'hardware': [{'frame': 9551, 'map': 19, 'x': 210, 'slot': 54}],
             'presented': [{'frame': 9553, 'map': 19, 'x': 210, 'slot': 54}]}
        m = correlate(p)['matches'][0]
        self.assertEqual(m['lagToPresentation'], 4)
        self.assertEqual(m['lagToHardware'], 2)
        p['hardware'][0]['x'] = 211
        self.assertEqual(correlate(p)['matches'][0]['status'], 'unresolved-no-hardware-match')

    def test_schema_and_required_fields(self):
        with self.assertRaises(ValueError):
            correlate({'schema': 'unexpected'})
        with self.assertRaises(ValueError):
            correlate({'schema': 'chq-sprite-pipeline-events-v1', 'prepared': [{'frame': 1}]})


if __name__ == '__main__':
    unittest.main()
