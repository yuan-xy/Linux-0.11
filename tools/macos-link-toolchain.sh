#!/usr/bin/env bash
#
# macOS toolchain compatibility helper for Linux-0.11
#
# This script creates i386-elf-* wrappers or symlinks based on
# Homebrew-provided i686-elf-* toolchain.
#
# Tested with:
#   - macOS (Apple Silicon / Intel)
#   - Homebrew i686-elf-binutils / i686-elf-gcc
#
# Author: Kiciot
#

set -euo pipefail

echo "== Linux-0.11 macOS toolchain compatibility script =="

# ------------------------------------------------------------
# 1. Detect Homebrew prefix
# ------------------------------------------------------------
if command -v brew >/dev/null 2>&1; then
    BREW_PREFIX="$(brew --prefix)"
else
    echo "[ERROR] Homebrew not found. Please install Homebrew first."
    exit 1
fi

BIN_DIR="$BREW_PREFIX/bin"

echo "[INFO] Homebrew prefix: $BREW_PREFIX"
echo "[INFO] Binary directory: $BIN_DIR"

# ------------------------------------------------------------
# 2. Tool mapping
# ------------------------------------------------------------
TOOLS=(
    gcc
    as
    ld
    ar
    ranlib
    strip
    objcopy
    nm
)

# cpp is special (often missing)
CPP_SRC="$BIN_DIR/i686-elf-cpp"
CPP_FALLBACK="$BIN_DIR/i686-elf-gcc"

# ------------------------------------------------------------
# 3. Link i686-elf-* -> i386-elf-*
# ------------------------------------------------------------
echo
echo "[STEP] Linking i686-elf-* -> i386-elf-*"

for tool in "${TOOLS[@]}"; do
    SRC="$BIN_DIR/i686-elf-$tool"
    DST="$BIN_DIR/i386-elf-$tool"

    if [ -x "$DST" ]; then
        echo "[SKIP] $DST already exists"
        continue
    fi

    if [ -x "$SRC" ]; then
        ln -sf "$SRC" "$DST"
        echo "[LINK] $DST -> $SRC"
    else
        echo "[WARN] Source tool not found: $SRC"
    fi
done

# ------------------------------------------------------------
# 4. Handle i386-elf-cpp (common pain point)
# ------------------------------------------------------------
echo
echo "[STEP] Handling i386-elf-cpp"

DST_CPP="$BIN_DIR/i386-elf-cpp"

if [ -x "$DST_CPP" ]; then
    echo "[SKIP] $DST_CPP already exists"
else
    if [ -x "$CPP_SRC" ]; then
        ln -sf "$CPP_SRC" "$DST_CPP"
        echo "[LINK] $DST_CPP -> $CPP_SRC"
    elif [ -x "$CPP_FALLBACK" ]; then
        echo "[INFO] i686-elf-cpp not found, creating wrapper using gcc -E"

        cat > "$DST_CPP" << 'EOF'
#!/usr/bin/env bash
exec i386-elf-gcc -E "$@"
EOF

        chmod +x "$DST_CPP"
        echo "[WRAP] $DST_CPP -> i386-elf-gcc -E"
    else
        echo "[ERROR] Neither i686-elf-cpp nor i686-elf-gcc found"
        echo "        Please install i686-elf-gcc via Homebrew."
        exit 1
    fi
fi

# ------------------------------------------------------------
# 5. Summary
# ------------------------------------------------------------
echo
echo "== Done =="
echo
echo "You should now have the following tools available:"
for tool in "${TOOLS[@]}"; do
    if command -v "i386-elf-$tool" >/dev/null 2>&1; then
        echo "  ✓ i386-elf-$tool"
    else
        echo "  ✗ i386-elf-$tool (missing)"
    fi
done

if command -v i386-elf-cpp >/dev/null 2>&1; then
    echo "  ✓ i386-elf-cpp"
else
    echo "  ✗ i386-elf-cpp (missing)"
fi

echo
echo "You can now run:"
echo "  make clean"
echo "  make -B"
echo
