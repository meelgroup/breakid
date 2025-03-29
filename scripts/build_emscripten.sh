set -e
rm -rf Make* cm* CM* lib* bliss breakid include src lib breakidConfig.cmake breakidTargets.cmake
emcmake cmake -DCMAKE_INSTALL_PATH=$EMINSTALL ..
emmake make -j6
emmake make install
