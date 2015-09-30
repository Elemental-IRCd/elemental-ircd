# GCC/Clang hardening rules

AC_DEFUN([HARDEN_RULES], [
    # Non-executable data
    AX_APPEND_LINK_FLAGS([-Wl,-z,noexecstack -Wl,-z,noexecheap])

    # Disable delayed symbol resolution
    AX_APPEND_LINK_FLAGS([-Wl,-z,now])
    # Mark some sections read-only after linking/dlopen
    AX_APPEND_LINK_FLAGS([-Wl,-z,relro])

    # Disable some maybe-unsafe optimizations
    AX_APPEND_COMPILE_FLAGS([-fno-strict-overflow])
    # This in particular we are violating
    AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing])
    # Better debug information
    AX_APPEND_COMPILE_FLAGS([-fno-omit-frame-pointer])
])
