# status
Status command for sway.

# Building

Build like a regular C program:

```
cc -o status status.c
```

# Setup

Edit your sway config to point to your newly built status command:

```
bar {
    # When the status_command prints a new line to stdout, swaybar updates.
    # The default just shows the current date and time.
    status_command while true; do /prg/stats; done

	# config for swaybar continues here...
}
```

(Replace `/path/to/status` with the actual path to the status executable)
