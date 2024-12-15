byte zdroj[8];

byte password[10];

struct MakeFile {
  byte* InputFileName;
  byte* OutputFileName;
  byte* Password;
  BOOL UseOutputFile;
  long DataCount;
};

struct Version {
  long MajorVersion;
  long MinorVersion;
  long Length;
  byte* Authors;
};
//
// authors
//
void Auth()
{
  Version* Vrs = new Version;
  Vrs->Length = 0;
  Authors(Vrs);
  Vrs->Authors = new byte[Vrs->Length];
  Authors(Vrs);
  MessageBox(NULL, Vrs->Authors, "Authors", MB_OK + MB_ICONINFORMATION);
  if (Vrs) delete Vrs;
}
//
// encrypt 8B
//
void En8B()
{
  byte* password = "0123456789";
  byte* source = "12345678";
  Encrypt8B(password, source);
  // source is encrypted
}
//
// decrypt 8B
//
void De8B()
{
  byte* password = "0123456789";
  byte* source = "øb#­¬S(}"
  Decrypt8B(password, zdroj);
  // source is decrypted
}
//
// encrypt file
//
void EnFile()
{
  MakeFile* Source = new MakeFile;
  // encrypt entire file
  Source->DataCount = 0;
  Source->Password = "0123456789";
  Source->InputFileName = "test.txt"
  Source->OutputFileName = "";
  Source->UseOutputFile = false;
  // encrypt
  long result = EncryptFile(Vstup);
  string mess;
  switch (result)
  {
    case -1: mess = "can't open input file"; break;
    case -2: mess = "can't open output file"; break;
    case -3: mess = "output file name have to be declared"; break;
    case -4: mess = "more memory needed"; break;
    default: mess = "encryption is done"; break;
  }
  MessageBox(NULL, mess, "info", MB_OK);
  if (Source) delete Source;
}
//
// decrypt file
//
void DeFile()
{
  MakeFile* Source = new MakeFile;
  // decrypt entire file
  Source->DataCount = 0;
  Source->Password = "0123456789";
  Source->InputFileName = "test.txt";
  Source->OutputFileName = "";
  Source->UseOutputFile = false;
  // encrypt
  long result = EncryptFile(Vstup);
  string mess;
  switch (result)
  {
    case -1: mess = "can't open input file"; break;
    case -2: mess = "can't open output file"; break;
    case -3: mess = "output file name have to be declared"; break;
    case -4: mess = "more memory needed"; break;
    default: mess = "decryption is done"; break;
  }
  MessageBox(NULL, mess, "info", MB_OK);
  if (Source) delete Source;
}
//
// version
//
void Ver()
{
  Version* Vrs = new Version;
  Vrs->Length = 0;
  Authors(Vrs);
  Vrs->Authors = new byte[Vrs->Length];
  Authors(Vrs);
  string Major = IntToStr(Vrs->MajorVersion);
  string Minor = IntToStr(Vrs->MinorVersion);
  string vers;
  vers = "version\n" + Major + "." + Minor;
  MessageBox(NULL, vers, "Version", MB_OK + MB_ICONINFORMATION);
  if (Vrs) delete Vrs;
}
