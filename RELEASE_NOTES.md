# Verbuno 0.3.2

This maintenance release prevents slow or unavailable inference endpoints from leaving Verbuno in the connecting state indefinitely and improves OpenRouter route selection without relaxing privacy controls.

## Highlights

- `openrouter/free` and exact `:free` models get one automatic second route when no model activity starts within a deadline beginning at 12 seconds, with extra allowance for long input;
- OpenRouter processing comments are treated only as status updates and cannot reset the first-token deadline;
- exact and custom models fail clearly after a bounded first-token wait instead of appearing frozen;
- a stream that stops producing text or reasoning activity is cancelled after 30 seconds, while already received text stays visible;
- streamed `reasoning` and `reasoning_details` chunks count as active work and receive a dedicated interface status;
- saved-key lookup has its own visible stage and an eight-second deadline, so a locked KWallet cannot leave the application waiting silently;
- OpenRouter routing now prefers low p90 latency and useful p50 throughput while retaining normal uptime-aware balancing and provider fallbacks;
- `data_collection: deny`, optional ZDR, redirect refusal and all existing credential protections remain unchanged;
- integration tests simulate SSE keep-alives, reasoning activity and a stalled partial response.

Free endpoints can still be rate-limited, temporarily unavailable or slow at peak times. Verbuno now bounds that wait and reports it accurately; it cannot make an external model available. For the most consistent response time, select an exact fast model instead of the random `openrouter/free` router.

## Release files

- Arch Linux `.pkg.tar.zst`
- Fedora 44 `.rpm`
- Ubuntu 24.04 `.deb`
- install tree `.tar.xz`
- source `.tar.xz`
- `PKGBUILD`
- SPDX 2.3 document
- `SHA256SUMS`

Every binary package is built and installed in its native target before publication. Verify downloaded files with:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

Translation text is sent to the configured external provider. Automatic retry is limited to the same explicitly requested free route and uses the same privacy restrictions. Review OpenRouter and the selected inference provider's current retention and training policies before submitting sensitive text.
