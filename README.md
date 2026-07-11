# NickelCloud

Copies (or syncs) books from cloud storage to your Kobo library using [rclone](https://rclone.org).

Books are copied when WiFi connection is established, and every 5 minutes whilst connected.

## Install

1. Download the `KoboRoot.tgz` found on the [latest release](https://github.com/btinworth/NickelCloud/releases/latest) page
2. Copy `KoboRoot.tgz` into your Kobo `.kobo` directory
3. Reboot your Kobo to install

## Configuration

Update the file `.adds/nickelcloud/rclone.conf` with your rclone configuration. Run `rclone config` on a computer to set up your remotes, then copy the contents shown by `rclone config show` into this file.

Edit `.adds/nickelcloud/nickelcloud.conf` and add one remote path per line under the `[sources]` section.
The remote paths follow the format of `remote:folder = local/folder`

For example, if `rclone.conf` contained a `[GoogleDrive]` entry, the following would copy the contents of the `Books` directory in Google Drive to the `GDriveBooks` directory on the Kobo:

```conf
[sources]
GoogleDrive:Books = GDriveBooks
```

The `[general]` section also has optional additional settings:

| Key | Default | Description |
| --- | --- | --- |
| `mode` | `copy` | `copy` to only add/update files, or `sync` to also delete local files removed from the source |
| `interval` | `300` | Seconds between checks, or `0` to only check once per connection |
| `transfers` | `1` | Number of concurrent file transfers |
| `log` | `false` | Write logs to `/mnt/onboard`, for debugging purposes |
| `extra_args` | | Extra arguments passed through to rclone as-is |

## Uninstall

To uninstall create a file named `uninstall` in the `.adds/nickelcloud` directory, then reboot the Kobo.

Uninstalling will not remove any downloaded books.

## Acknowledgements

Uses [NickelHook](https://github.com/pgaskin/NickelHook) and [rclone](https://github.com/rclone/rclone).

Inspired by the [KoboClone](https://github.com/fsantini/KoboCloud) project.
