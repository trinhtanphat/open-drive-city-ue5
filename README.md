# Open Drive City UE5

Source-first Unreal Engine 5 driving sandbox.

Phase 1 is developed and statically verified on VPS 182. Runtime/editor GPU validation is intentionally deferred until a discrete-GPU workstation is available.

## Current status

- UE5 C++ project skeleton
- Chaos Vehicles enabled
- Enhanced Input enabled
- No proprietary or Marketplace assets committed
- Source/static verification only on VPS 182

## Verification

```powershell
python -m unittest discover -s tests -v
python tools/verify_repo.py
```

Project-owned source is MIT licensed. Unreal Engine and third-party assets remain under their own licenses.
