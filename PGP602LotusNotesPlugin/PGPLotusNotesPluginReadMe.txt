          PGP Lotus Notes Plugin ReadMe
  Copyright (c) 1999 by Network Associates, Inc., 
 and its Affiliated Companies. All Rights Reserved.


Thank you for using Network Associates' products. This 
ReadMe file contains important information regarding 
the PGP Lotus Notes Plugin. Network Associates strongly
recommends that you read this entire document.

Network Associates welcomes your comments and suggestions.
Please use the information provided in this file to 
contact us.

Warning: Export of this software may be restricted by the
U.S. Government.

WHAT'S IN THIS FILE

- System requirements
- Installation
- Using the PGP Lotus Notes Plugin
- Known Issues

___________________
SYSTEM REQUIREMENTS

To run the PGP Lotus Notes plugin, you must have:

- Lotus Notes software versions 4.5x or 4.6x for clients. 
  Lotus Domino software versions 4.5x or 4.6x for servers.

- PGP version 6.0.2.

____________
INSTALLATION

Please refer to the PGP Lotus Notes Plugin Installation 
Guide located on the CD in Network Associates\PGP Lotus 
Notes Plugin\PGPLotusNotesPluginInstallGuide.pdf.

____________________________
USING THE LOTUS NOTES PLUGIN

To encrypt and sign an e-mail message, follow these steps:

  1. Compose your e-mail message as you normally would.

  2. When you have finished composing the text of your 
     e-mail message, click the Encrypt button and the 
     Sign button in the upper right hand corner of your
     message. 

  3. Send your message as you normally do.
   
     If you have a copy of the public keys for every 
     one of the recipients, the appropriate keys are used.
     However, if you specify a recipient for whom there
     is no corresponding public key or if one or more of 
     the keys have insufficient validity, the PGP Key
     Selection dialog box appears so that you can specify
     the correct key.

  4. Drag the public keys for those who are to receive a
     copy of the encrypted e-mail message into the 
     Recipients list box. You can also double-click any
     of the keys to move them from one area of the screen
     to the other.

  5. Select the Conventional Encrypt option to use a common
     passphrase instead of public key encryption. If you 
     select this option, the file is encrypted using 
     a session key, which encrypts (and decrypts)
     using a passphrase that you will be asked to choose.

  6. Click OK to encrypt and sign your mail.

     If you have elected to sign the encrypted data, the 
     Signing Key Passphrase dialog box appears requesting 
     your passphrase before the mail is sent.

  7. Enter your passphrase and then click OK.
 
    
To encrypt a message with an attached file, follow these 
steps:

  1. Attach the file to the message as you normally would.

  2. Follow the instructions outlined above for encrypting and
     signing e-mail messages.
 

To decrypt and verify an e-mail message, follow these steps:

  1. Open your e-mail message as you normally would.
  
     If your PGP e-mail preferences are set to decrypt e-mail 
     automatically, the Enter Passphrase dialog box appears.
     Enter your passphrase and click OK. The message is 
     decrypted and appears in plain text. Otherwise, you 
     will see a block of unintelligible ciphertext in the
     body of your e-mail message.

  2. To decrypt the ciphertext, do one of the following:
  
      Click the Decrypt/Verify button on the Lotus Notes
       toolbar.
     OR  
      Select the PGP Decrypt/Verify option listed in the
       Actions menu.

     The PGP Enter Passphrase dialog box appears asking you to
     enter your passphrase.
 
  4. Enter your passphrase, then click OK.
   
     The message is decrypted. If it has been signed and 
     you have the senders' public key, a message appears 
     indicating whether the signature is valid.

  5. If you wish to save the message in its decrypted 
     state, you must enter edit mode by double-clicking 
     somewhere within the message or pressing <Ctrl>+e 
     or choosing the Edit Document item under the Actions 
     menu. Once in edit mode, you can save the document 
     by pressing <Ctrl>+s, pressing the "Save and File" 
     action button or choosing the Save item from the 
     File menu.

 
To decrypt an attached file, follow these steps:

  1. Double-click the encrypted .asc file in your e-mail 
     message. 

     The Properties for Attachment dialog appears.  
 
  2. Select one of these options:
     
      View. Select this option to open the attached file
       and view its encrypted text.

      Launch. Select this option to decrypt the file. The
       encrypted file is copied to your TEMP directory
       (e.g. C:\TEMP), then a decrypted version of the file
       is created in your TEMP directory. To retrieve the
       decrypted file, browse to the file (with Windows
       Explorer, for instance).
    
      Detach. Select this option to save the attached file
       in its encrypted state on your computer using the 
       standard File Save dialog box.

____________
KNOWN ISSUES

1. When encrypting a message using conventional encryption,
   the "Enter Passphrase" dialog box appears multiple times.

2. Encrypt and Sign functions do not appear in the Lotus 
   Notes "Actions" menu, but Decrypt and Verify do appear. 
   Also, the Encrypt and Sign buttons appear in the upper 
   right side of the message and not in the toolbar.

