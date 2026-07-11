# NickelCloud

Pull books from cloud storage to your Kobo library using [rclone](https://rclone.org).

## Install

1. Download the latest `KoboRoot.tgz` from the latest release:
   <https://github.com/btinworth/NickelCloud/releases/latest>
2. Copy `KoboRoot.tgz` into your Kobo `.kobo` directory
3. Reboot your Kobo to install
4. Create an rclone config on your computer (`rclone config`) and copy it to:
   `.adds/nickelcloud/rclone.conf`
5. Edit `.adds/nickelcloud/nickelcloud.conf` and add one remote path per line.
   For example, the following would copy the contents of the `Books` directory in Google Drive to the `GDriveBooks` directory

   ```conf
   GoogleDrive:Books = GDriveBooks
   ```

## Uninstall

To uninstall create a file named `uninstall` in the `.adds/nickelcloud` directory, then reboot the Kobo.
Uninstalling will not remove any downloaded books.

## Acknowledgements

* The original inspiration for this came from the [KoboClone](https://github.com/fsantini/KoboCloud) project
* Uses [NickelHook](https://github.com/pgaskin/NickelHook)
* Uses [rclone](https://github.com/rclone/rclone)
