#!/usr/bin/env bash
# Run a command (default: interactive shell) inside the Rastera dev container.
#   ./dev.sh                 -> shell
#   ./dev.sh build           -> configure + build into build/
#   ./dev.sh test            -> build, then run the test suite
#   ./dev.sh <any command>   -> run it in the container
set -euo pipefail

IMAGE=rastera-dev
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    docker build -t "$IMAGE" "$ROOT"
fi

case "${1:-}" in
    build) set -- bash -c 'cmake -S . -B build -G Ninja && cmake --build build' ;;
    test)  set -- bash -c 'cmake -S . -B build -G Ninja && cmake --build build && tests/run_tests.sh build/rasterc' ;;
    "")    set -- bash ;;
esac

TTY=()
[ -t 0 ] && TTY=(-it)
exec docker run --rm "${TTY[@]}" -u "$(id -u):$(id -g)" -v "$ROOT:/work" -w /work "$IMAGE" "$@"
