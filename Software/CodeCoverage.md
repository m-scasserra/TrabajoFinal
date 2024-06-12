PIO terminal desde /Software

Para generar info
lcov --ignore-errors inconsistent --directory .pio/build/native/test/test_native --capture --output-file coverage/coverage.info

Para generar HTML
genhtml coverage/coverage.info --output-directory coverage/