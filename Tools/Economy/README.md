# Economy Reference Tooling

Pure reference semantics for atomic Torque Atlas economy/inventory transactions.

The runtime/server implementation must preserve these invariants:

- transaction IDs apply at most once;
- insufficient credits fail before mutation;
- owned inventory cannot be removed twice;
- duplicate inventory cannot be added silently;
- debit/credit + inventory mutation commit atomically.

This tool is not the production database. It is a deterministic oracle for save/server tests.
