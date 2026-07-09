# kobo-rclone

Pulls eBooks from the cloud to your Kobo library using [rclone](https://rclone.org).

## Install

1. Download the latest `KoboRoot.tgz` from the latest release:
   <https://github.com/btinworth/kobo-rclone/releases/latest>
2. Copy `KoboRoot.tgz` into your Kobo `.kobo` directory.
3. Reboot the Kobo.
4. Create your rclone config on your computer (`rclone config`) and copy it to:
   `.adds/koborclone/rclone.conf`
5. Edit `.adds/koborclone/koborclone.conf` and add one remote path per line, for example:

   ```conf
   GoogleDrive:Books
   Dropbox:ebooks
   ```

## Uninstall

To uninstall create a file named `UNINSTALL` in the `.adds/koborclone` directory, then reboot the Kobo.
Uninstalling will not remove any downloaded books.

## Acknowledgements

* Based on [KoboClone](https://github.com/fsantini/KoboCloud) by fsantini
* Installs [NickelDBus](https://github.com/shermp/NickelDBus) by shermp if missing
* Depends largely on [rclone](https://github.com/rclone/rclone)
