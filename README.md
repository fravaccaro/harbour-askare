# Askare

A Nextcloud task client for Sailfish OS. Sign in with the server address, finish
the login in the app, then work through CalDAV task lists on the device.

Askare does not use the Nextcloud account already in Settings. The system
calendar only syncs events, so this app keeps its own copy of the tasks.

## Sign in

1. Enter the Nextcloud URL.
2. Sign in on the page that opens, including single sign-on or two-factor authentication.
3. Nextcloud creates a password named Askare. Revoke it later under Settings, Security.

## Build

```
sfdk config target SailfishOS-5.0.0.62-aarch64
sfdk qmake
sfdk make
sfdk rpm
```

Needs Sailfish OS 4.5 or newer for Sailfish Secrets and the WebView.

## Harbour

The package links only Qt, Silica, the WebView, and Sailfish Secrets. Sync is
manual. The system calendar is not opened.
