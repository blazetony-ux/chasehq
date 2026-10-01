"""Generate the offline handbook from the same authored sources as Workbench."""
from pathlib import Path
import html
import json
import re

root = Path(__file__).resolve().parents[1]
version = re.search(r'kNativeVersion\s*=\s*"([^"]+)"', (root / 'src/version.h').read_text()).group(1)
handbook = json.loads((root / 'research/knowledge/handbook.json').read_text())
knowledge = json.loads((root / 'research/knowledge/documentation.json').read_text())
api = json.loads((root / 'research/schema/api-actions.json').read_text())
esc = lambda value: html.escape(str(value))
body, nav = [], []

def table(headers, rows):
    return '<table><thead><tr>' + ''.join('<th>' + esc(x) + '</th>' for x in headers) + '</tr></thead><tbody>' + ''.join('<tr>' + ''.join('<td>' + esc(x) + '</td>' for x in row) + '</tr>' for row in rows) + '</tbody></table>'

for section in handbook['sections']:
    nav.append('<a href="#' + esc(section['id']) + '">' + esc(section['title']) + '</a>')
    body.append('<section id="' + esc(section['id']) + '"><h2>' + esc(section['title']) + '</h2>')
    for page in section['pages']:
        body.append('<article id="' + esc(page['id']) + '"><h3>' + esc(page['title']) + '</h3><p>' + esc(page.get('summary', '')) + '</p>')
        for text in page.get('body', []):
            body.append('<p>' + esc(text) + '</p>')
        for code in page.get('examples', []):
            body.append('<pre>' + esc(code) + '</pre>')
        for block in page.get('blocks', []):
            kind = block['type']
            if kind == 'table':
                body.append(table(block['headers'], block['rows']))
            elif kind in ('steps', 'list'):
                tag = 'ol' if kind == 'steps' else 'ul'
                body.append('<' + tag + '>' + ''.join('<li>' + esc(x) + '</li>' for x in block.get('items', [])) + '</' + tag + '>')
            elif kind == 'code':
                body.append('<pre>' + esc(block.get('text', block.get('code', ''))) + '</pre>')
            else:
                body.append('<div class="' + esc(kind) + '">' + ('<b>' + esc(block['title']) + '</b>' if block.get('title') else '') + '<p>' + esc(block.get('text', '')) + '</p></div>')
        body.append('</article>')
    body.append('</section>')

body.append('<section id="api"><h2>Current API contracts</h2>' + table(['Action', 'Parameters', 'Example', 'Meaning'], [[name, '; '.join(c['params']), c['example'], c['description']] for name, c in api['actions'].items()]) + '</section>')
nav.append('<a href="#api">Current API contracts</a>')
body.append('<section id="knowledge"><h2>Knowledge and findings</h2>')
for entry in knowledge['entries']:
    body.append('<article><h3>' + esc(entry['title']) + '</h3><small>' + esc(entry['status']) + ' · ' + esc(entry['confidence']) + '</small><p>' + esc(entry['summary']) + '</p>')
    for label in ('docs', 'scripts', 'apiActions'):
        if entry.get(label):
            body.append('<p><b>' + esc(label) + ':</b> ' + esc(', '.join(entry[label])) + '</p>')
    if entry.get('article'):
        body.append('<details><summary>Source details</summary><pre>' + esc(json.dumps(entry['article'], indent=2)) + '</pre></details>')
    body.append('</article>')
body.append('</section>')
nav.append('<a href="#knowledge">Knowledge and findings</a>')
css = 'body{margin:0;background:#0b1018;color:#e8f0fa;font:15px/1.55 system-ui}header{padding:32px;background:#152236;border-bottom:8px solid #e13039}header small{color:#85ccff}.layout{display:grid;grid-template-columns:240px minmax(0,1fr)}nav{padding:20px;position:sticky;top:0;height:90vh;overflow:auto}nav a{display:block;color:#8ed2ff;margin:12px 0}main{padding:24px;max-width:1200px}article{padding:18px;margin:16px 0;background:#121c29;border:1px solid #2c4058}h2{color:#8ed2ff}pre{white-space:pre-wrap;overflow-wrap:anywhere;background:#080d14;padding:12px}table{border-collapse:collapse;width:100%;font-size:12px}td,th{padding:8px;border:1px solid #33465d;text-align:left;overflow-wrap:anywhere}.callout{border-left:4px solid #e13039;padding:12px;background:#22202a}small{color:#a9bacd}@media(max-width:850px){.layout{display:block}nav{position:static;height:auto}table{display:block;overflow:auto}}'
out = '<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>ChaseHQ-Native Handbook ' + esc(version) + '</title><style>' + css + '</style><header><h1>CHASE H.Q. — Native Research Handbook</h1><small>' + esc(version) + ' · SOURCE CANDIDATE · Windows promotion pending</small></header><div class="layout"><nav>' + ''.join(nav) + '</nav><main>' + ''.join(body) + '</main></div></html>'
target = root / 'docs/handbook/ChaseHQ-Native-Documentation-Handbook.html'
target.write_text(out)
print('Generated', target.relative_to(root))
