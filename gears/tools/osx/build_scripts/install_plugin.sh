# This script is run from the gears/tools/osx directory

# clean up any previous products/symbolic links in the Internet Plug-Ins folder
if [ -a "${USER_LIBRARY_DIR}/Internet Plug-Ins/${FULL_PRODUCT_NAME}" ]; then
  rm -Rf "${USER_LIBRARY_DIR}/Internet Plug-Ins/${FULL_PRODUCT_NAME}"
fi

# Copy the plugin to the Internet Plug-ins folder
cp -Rfv "${TARGET_BUILD_DIR}/${FULL_PRODUCT_NAME}" \
   "${USER_LIBRARY_DIR}/Internet Plug-Ins/${FULL_PRODUCT_NAME}"