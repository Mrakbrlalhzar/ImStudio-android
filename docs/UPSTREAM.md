# Upstream sync

The file `.upstream-commit` records the upstream SHA from the last successful sync.

Vendor files (`android/app/src/main/cpp/vendor/`) are always refreshed
from upstream. To force-refresh locally:

```bash
./scripts/sync-upstream.sh
```
