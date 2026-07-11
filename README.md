# NickelCloud

NickelCloud pulls books from cloud storage to your Kobo library using [rclone](https://rclone.org).

It runs automatically when WiFi connects, then checks again every 5 minutes while WiFi stays connected.

## Installation

1. Download `KoboRoot.tgz` from the the [releases page](https://github.com/btinworth/NickelCloud/releases/latest).
2. Copy `KoboRoot.tgz` into the `.kobo` folder on your Kobo.
3. Reboot your Kobo.

After reboot, NickelCloud creates its config files in the `.adds/nickelcloud` folder.

## Configuration

### 1) Add your rclone config

On your computer:

1. Run `rclone config` to set up your cloud remote.
2. Run `rclone config show`.
3. Copy that output into the `.adds/nickelcloud/rclone.conf` file.

### 2) Choose what to sync

Open the `.adds/nickelcloud/nickelcloud.conf` file in a text editor.

Under `[sources]`, add one mapping per line:

`remote:path = local/folder`

For example, this syncs Google Drive `Books` into `/mnt/onboard/GDriveBooks`.

```conf
[sources]
GoogleDrive:Books = GDriveBooks
```

## Settings

In `[general]` inside `nickelcloud.conf`:

| Key | Default | Description |
| --- | --- | --- |
| `mode` | `copy` | `copy` adds/updates files. `sync` also deletes local files removed from cloud. |
| `interval` | `300` | Seconds between checks while WiFi is connected. Use `0` to run only once per connection. |
| `transfers` | `1` | Number of simultaneous file transfers. |
| `log` | `false` | If `true`, writes logs to `/mnt/onboard` when changes/failures occur. |
| `extra_args` | (empty) | Extra rclone flags, space-separated. |

## Uninstall

1. Create an empty file named `uninstall` in `.adds/nickelcloud`.
2. Reboot your Kobo.

Downloaded books are not removed.

## Credits

Built with [NickelHook](https://github.com/pgaskin/NickelHook) and [rclone](https://github.com/rclone/rclone).
Inspired by [KoboCloud/KoboClone](https://github.com/fsantini/KoboCloud).
