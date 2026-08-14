# Safety Model

StayInk may eventually interact with system-level device behavior, so safety work begins before implementation.

1. Research the relevant behavior before proposing a device modification.
2. Use read-only device probing before any write operation.
3. Prefer reversible, runtime-only experiments before persistent changes.
4. Maintain an explicit support matrix by Kindle model and firmware. Evidence from one combination does not establish support for another.
5. Require a fail-safe or disable mechanism before any release that changes device behavior.
6. Design and verify uninstall and recovery procedures before release.

Any future device-changing experiment must define its scope, observable success criteria, risks, rollback steps, and recovery path before it runs. Kindle-specific mechanisms remain unestablished until supported by recorded evidence.
