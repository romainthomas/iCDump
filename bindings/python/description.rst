Introduction
============

This project aims at providing a cross-platform utility to dump Objective-C and Swift metadata
from 64-bits Mach-O binaries.

Getting Started
===============

.. code-block:: python

   import icdump
   metadata = icdump.objc.parse("./RNCryptor.bin")

   # Iterate over all the Objective-C interfaces (classes)
   for cls in metadata.classes:
       print(cls.demangled_name)

.. code-block:: console

  PodsDummy_RNCryptor_iOS
  RNCryptor.RNCryptor.Encryptor
  RNCryptor.RNCryptor.Decryptor
  RNCryptor.RNCryptor.FormatV3
  RNCryptor.RNCryptor.EncryptorV3
  RNCryptor.RNCryptor.DecryptorV3
  RNCryptor.Engine
  RNCryptor.(DecryptorEngineV3 in _FFCD8353248C1C3EE416689E2680CC5C)
  RNCryptor.(HMACV3 in _FFCD8353248C1C3EE416689E2680CC5C)
  RNCryptor.OverflowingBuffer

.. code-block:: python

  # Dump the metadata as a header declaration
  print(metadata.to_decl())

.. code-block:: console

  @interface RNCryptor.RNCryptor.Encryptor{
      NSObject * encryptor;
  }
  @end
  @interface RNCryptor.RNCryptor.Decryptor{
      NSObject * decryptors;
      NSObject * buffer;
      NSObject * decryptor;
      NSObject * password;
  }
  @end
  ...
