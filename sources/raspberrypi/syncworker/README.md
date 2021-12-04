**RClone howto(s):**

    How to create a GDrive remote:
        > run "rclone config" -> choose "new remote" -> enter new remote name -> choose "Google Drive" type
        > client_id             -> default (empty)
        > client_secret         -> default (empty)
        > scope                 -> 2 (Read-only access)
        > root_folder_id        -> default (empty)
        > service_account_file  -> default (empty)
        > Edit advanced config? -> default (No)
        > Use auto config?      => NO (Say N if you are working on a remote or headless machine)
           > copy/paste url in a browser and authorize rclone access
           > Enter verification code   -> paste received code from browser
        > Configure this as a Shared Drive (Team Drive)? -> default (No)
        > y) Yes this is OK (default)
        
    Sync GDrive remote to local:
        rclone sync -P <remote_id>:/ /path/to/local/storage --log-file=path/to/logfile.txt