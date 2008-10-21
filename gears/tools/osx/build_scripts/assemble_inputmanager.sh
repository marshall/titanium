# Build the input manager packaging

INPUT_MGR_DIR="${BUILD_DIR}/${BUILD_STYLE}/GearsEnabler"
mkdir -p "$INPUT_MGR_DIR"
cp -fR "$BUILD_DIR/$BUILD_STYLE/$WRAPPER_NAME" "$INPUT_MGR_DIR"
cp -fR "$SRCROOT/Info" "$INPUT_MGR_DIR"

echo generated inputmanager @ "$INPUT_MGR_DIR"
