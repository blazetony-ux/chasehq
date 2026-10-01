# Discovery record — road geometry and Stage-1 topology

**Status:** core road/course channels CONFIRMED; fork/selector lifecycle BEHAVIOUR UNDERSTOOD; directed map representation outstanding.

## Live topology state

- page/index: CPU-A `0x10A058`
- selector/table phase: `0x10A059`
- downstream mirror: `0x10A018/19`
- player lateral coordinate used by fork choice: `0x10A044`

The branch routine uses a 32-byte table at ROM `0xAF64 + selector*32`, indexes the current page by `page*2`, and selects candidate A/B by whether player lateral is below/above `0x8000` when the pair differs. The A/B labels are structural; do not rename them physical left/right without separate proof.

## Valid selector tables

Only selectors 0-9 are valid transition tables. Selectors 0-4 share the branching template; selectors 5-9 share a linear `0 -> 1 -> 2 -> ... -> 15 -> 0` template. On terminal `page 15 -> 0`, code at `0x8088` adds 5 to selectors below 5, producing pairings `0->5`, `1->6`, `2->7`, `3->8`, `4->9`.

Causal branch proof with selector 0:

- lateral `0x7F00`: `0 -> 1 -> 3 -> 7`
- lateral `0x8100`: `0 -> 1 -> 4 -> 9`

Full lifecycle proof observed `0 -> 1 -> 3 -> 7 -> 15`, then selector `0 -> 5` and page `15 -> 0`. Do not spend further experiments reproving this low-level mechanism.

## Next step

Represent these proven transitions as an authoritative directed course graph/minimap and add semantic route labels only where supported by evidence.
