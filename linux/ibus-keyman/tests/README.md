# ibus-keyman integration tests

The ibus-keyman integration tests run the tests defined in `common/core/desktop/tests/unit/kmx`.
The tests run in a separate Xephyr session. Any dconf settings that have to be set during the tests
are stored in `~/.config/glib-2.0/settings/keyfile`.

## Running tests

The tests get run as part of building `ibus-keyman`, more specifically when running `make check`.

### Run all tests

All tests can be run with the test script:

```bash
./run-tests.sh
```

### Run specific tests

To run a single test you pass the testname (as found in
`common/core/desktop/build/arch/*/tests/unit/kmx`). Multiple tests should be separated by space.

```bash
./run-tests.sh -- k_000___null_keyboard k_005___nul_with_initial_context
```
