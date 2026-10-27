---
layout: default
title: Signing in
parent: Using Askare
nav_order: 1
---

# Signing in

Enter the address of your Nextcloud server, for example `https://cloud.example.org`, and tap **Sign in**. The sign-in page of the server then opens inside Askare: sign in there as you would in a browser. Single sign-on and two-factor authentication work, because whatever the page accepts is what grants access.

Once the server grants access it creates an app password named Askare, and Askare keeps that. Your own password is never asked for and never stored. The app password can be revoked at any time on the server, under *Settings → Security*; Askare then asks to sign in again.

The app password is kept in **Sailfish Secrets**, in a collection locked with the device lock, not in a plain file.

## Self-signed certificates

A server whose certificate the device does not trust cannot be reached until **Ignore certificate errors** is turned on. The switch sits on the sign-in page and, afterwards, on the settings page. Leave it off for a server with a certificate the device already trusts.

## When the password cannot be read

Askare waits for Sailfish Secrets before it shows anything. If the secure storage cannot be read - usually because the device lock has not been entered since the phone started - the app says so and offers **Try again** instead of the task list.

## Signing out

**Sign out**, on the settings page, removes the app password from Sailfish Secrets and empties the local copy of the lists and tasks. Anything not yet uploaded is lost with it, so sync first if a change is still waiting. The app password itself remains listed on the server until you revoke it there.
