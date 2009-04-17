/**@file
  This file implements the protocol functions related to string package.
  
Copyright (c) 2006 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "HiiDatabase.h"

typedef struct {
  CHAR8 *Iso639;
  CHAR8 *Rfc3066;
} ISO639TORFC3066MAP;

ISO639TORFC3066MAP Iso639ToRfc3066Map [] = {
    {"eng", "en-US"},
    {"fra", "fr-FR"},
};

//
// Lookup table of ISO639-2 3 character language codes to ISO 639-1 2 character language codes
// Each entry is 5 CHAR8 values long.  The first 3 CHAR8 values are the ISO 639-2 code.
// The last 2 CHAR8 values are the ISO 639-1 code.
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8 Iso639ToRfc3066ConversionTable[] =
"\
aaraa\
abkab\
afraf\
amham\
araar\
asmas\
aymay\
azeaz\
bakba\
belbe\
benbn\
bihbh\
bisbi\
bodbo\
brebr\
bulbg\
catca\
cescs\
corkw\
cosco\
cymcy\
danda\
deude\
dzodz\
ellel\
engen\
epoeo\
estet\
euseu\
faofo\
fasfa\
fijfj\
finfi\
frafr\
fryfy\
gaiga\
gdhgd\
glggl\
grngn\
gujgu\
hauha\
hebhe\
hinhi\
hrvhr\
hunhu\
hyehy\
ikuiu\
ileie\
inaia\
indid\
ipkik\
islis\
itait\
jawjw\
jpnja\
kalkl\
kankn\
kasks\
katka\
kazkk\
khmkm\
kinrw\
kirky\
korko\
kurku\
laolo\
latla\
lavlv\
linln\
litlt\
ltzlb\
malml\
marmr\
mkdmk\
mlgmg\
mltmt\
molmo\
monmn\
mrimi\
msams\
myamy\
nauna\
nepne\
nldnl\
norno\
ocioc\
ormom\
panpa\
polpl\
porpt\
pusps\
quequ\
rohrm\
ronro\
runrn\
rusru\
sagsg\
sansa\
sinsi\
slksk\
slvsl\
smise\
smosm\
snasn\
sndsd\
somso\
sotst\
spaes\
sqisq\
srpsr\
sswss\
sunsu\
swasw\
swesv\
tamta\
tattt\
telte\
tgktg\
tgltl\
thath\
tsnts\
tuktk\
twitw\
uigug\
ukruk\
urdur\
uzbuz\
vievi\
volvo\
wolwo\
xhoxh\
yidyi\
zhaza\
zhozh\
zulzu\
";

CHAR8 *
ConvertIso639ToRfc3066 (
  CHAR8 *Iso638Lang
  )
{
  UINTN Index;
  CHAR8 AsciiLanguage[ISO_639_2_ENTRY_SIZE + 1];
  
  AsciiStrnCpy (AsciiLanguage, Iso638Lang, sizeof (AsciiLanguage));
  for (Index = 0; Index < ISO_639_2_ENTRY_SIZE + 1; Index ++) {
  	if (AsciiLanguage [Index] == 0) {
  		break;
  	} else if (AsciiLanguage [Index] >= 'A' && AsciiLanguage [Index] <= 'Z') {
  		AsciiLanguage [Index] = (CHAR8) (AsciiLanguage [Index] - 'A' + 'a');
  	}
  }

  for (Index = 0; Index < sizeof (Iso639ToRfc3066Map) / sizeof (Iso639ToRfc3066Map[0]); Index++) {
    if (AsciiStrnCmp (AsciiLanguage, Iso639ToRfc3066Map[Index].Iso639, AsciiStrSize (AsciiLanguage)) == 0) {
      return Iso639ToRfc3066Map[Index].Rfc3066;
    }
  }

  return (CHAR8 *) NULL;
}

/**
  Convert language code from RFC3066 to ISO639-2.

  @param  LanguageRfc3066        RFC3066 language code.
  @param  LanguageIso639         ISO639-2 language code.

  @retval EFI_SUCCESS            Language code converted.
  @retval EFI_NOT_FOUND          Language code not found.

**/
EFI_STATUS
EFIAPI
ConvertRfc3066LanguageToIso639Language (
  IN  CHAR8   *LanguageRfc3066,
  OUT CHAR8   *LanguageIso639
  )
{
  UINTN  Index;

  if ((LanguageRfc3066[2] != '-') && (LanguageRfc3066[2] != 0)) {
    CopyMem (LanguageIso639, LanguageRfc3066, 3);
    return EFI_SUCCESS;
  }

  for (Index = 0; Iso639ToRfc3066ConversionTable[Index] != 0; Index += 5) {
    if (CompareMem (LanguageRfc3066, &Iso639ToRfc3066ConversionTable[Index + 3], 2) == 0) {
      CopyMem (LanguageIso639, &Iso639ToRfc3066ConversionTable[Index], 3);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}


/**
  Convert language code from ISO639-2 to RFC3066 and return the converted language.
  Caller is responsible for freeing the allocated buffer.

  LanguageIso639 contain a single ISO639-2 code such as
  "eng" or "fra".

  If LanguageIso639 is NULL, then ASSERT.
  If LanguageRfc3066 is NULL, then ASSERT.

  @param  LanguageIso639         ISO639-2 language code.

  @return the allocated buffer or NULL, if the language is not found.

**/
CHAR8*
EFIAPI
ConvertIso639LanguageToRfc3066Language (
  IN  CONST CHAR8   *LanguageIso639
  )
{
  UINTN Index;
  CHAR8 *Rfc3066Language;
  
  for (Index = 0; Iso639ToRfc3066ConversionTable[Index] != 0; Index += 5) {
    if (CompareMem (LanguageIso639, &Iso639ToRfc3066ConversionTable[Index], 3) == 0) {
      Rfc3066Language = AllocateZeroPool (3);
      if (Rfc3066Language != NULL) {
        Rfc3066Language = CopyMem (Rfc3066Language, &Iso639ToRfc3066ConversionTable[Index + 3], 2);
      }
      return Rfc3066Language;
    }
  }

  return NULL;
}

/**
  Get next language from language code list (with separator ';').

  If LangCode is NULL, then ASSERT.
  If Lang is NULL, then ASSERT.

  @param  LangCode    On input: point to first language in the list. On
                                 output: point to next language in the list, or
                                 NULL if no more language in the list.
  @param  Lang           The first language in the list.

**/
VOID
EFIAPI
GetNextLanguage (
  IN OUT CHAR8      **LangCode,
  OUT CHAR8         *Lang
  )
{
  UINTN  Index;
  CHAR8  *StringPtr;

  ASSERT (LangCode != NULL);
  ASSERT (*LangCode != NULL);
  ASSERT (Lang != NULL);

  Index = 0;
  StringPtr = *LangCode;
  while (StringPtr[Index] != 0 && StringPtr[Index] != ';') {
    Index++;
  }

  CopyMem (Lang, StringPtr, Index);
  Lang[Index] = 0;

  if (StringPtr[Index] == ';') {
    Index++;
  }
  *LangCode = StringPtr + Index;
}

/**
  Test if all of the characters in a string have corresponding font characters.

  This is a deprecated API. No Framework HII module is calling it. This function will ASSERT and
  return EFI_UNSUPPORTED.

  @param This            A pointer to the EFI_HII_PROTOCOL instance.
  @param StringToTest    A pointer to a Unicode string.
  @param FirstMissing    A pointer to an index into the string. On input, the index of 
                         the first character in the StringToTest to examine. On exit, the index 
                         of the first character encountered for which a glyph is unavailable. 
                         If all glyphs in the string are available, the index is the index of the terminator 
                         of the string. 
  @param GlyphBufferSize A pointer to a value. On output, if the function returns EFI_SUCCESS, 
                         it contains the amount of memory that is required to store the string? glyph equivalent.

  @retval EFI_UNSUPPORTED  The function performs nothing and return EFI_UNSUPPORTED.
**/
EFI_STATUS
EFIAPI
HiiTestString (
  IN     EFI_HII_PROTOCOL   *This,
  IN     CHAR16             *StringToTest,
  IN OUT UINT32             *FirstMissing,
  OUT    UINT32             *GlyphBufferSize
  )
{
  ASSERT (FALSE);
  
  return EFI_UNSUPPORTED;
}


/**
  Find the corressponding TAG GUID from a Framework HII Handle given.

  @param Private      The HII Thunk Module Private context.
  @param FwHiiHandle  The Framemwork HII Handle.
  @param TagGuid      The output of TAG GUID found.

  @return NULL        If Framework HII Handle is invalid.
  @return The corresponding HII Thunk Context.
**/
EFI_STATUS
GetTagGuidByFwHiiHandle (
  IN  CONST HII_THUNK_PRIVATE_DATA      *Private,
  IN        FRAMEWORK_EFI_HII_HANDLE    FwHiiHandle,
  OUT       EFI_GUID                    *TagGuid
  )
{
  LIST_ENTRY                                *Link;
  HII_THUNK_CONTEXT                          *ThunkContext;

  ASSERT (TagGuid != NULL);

  Link = GetFirstNode (&Private->ThunkContextListHead);
  while (!IsNull (&Private->ThunkContextListHead, Link)) {

    ThunkContext = HII_THUNK_CONTEXT_FROM_LINK (Link);

    if (FwHiiHandle == ThunkContext->FwHiiHandle) {
      CopyGuid (TagGuid, &ThunkContext->TagGuid);
      return EFI_SUCCESS;
    }

    Link = GetNextNode (&Private->ThunkContextListHead, Link);
  }

  return EFI_NOT_FOUND;
}

/**
  Create or update the String given a new string and String ID.

  @param ThunkContext           The Thunk Context.
  @param Rfc3066AsciiLanguage   The RFC 3066 Language code in ASCII string format.
  @param NewString              The new string.
  @param StringId               The String ID. If StringId is 0, a new String Token
                                is created. Otherwise, the String Token StringId is 
                                updated.
                                

  @retval EFI_SUCCESS           The new string is created or updated successfully. 
                                The new String Token ID is returned in StringId if
                                *StringId is 0 on input.
  @return Others                The update of string failed.                                  
  
**/
EFI_STATUS
UpdateString (
  IN CONST HII_THUNK_CONTEXT        *ThunkContext,
  IN CONST CHAR8                    *Rfc3066AsciiLanguage,
  IN       CHAR16                   *NewString,
  IN OUT STRING_REF                 *StringId
  )
{
  EFI_STRING_ID                             NewStringId;

  NewStringId = HiiSetString (ThunkContext->UefiHiiHandle, *StringId, NewString, Rfc3066AsciiLanguage);
  *StringId = NewStringId;
  if (NewStringId == 0) {
    //
    // Only EFI_INVALID_PARAMETER is defined in HII 0.92 specification.
    //
    return EFI_INVALID_PARAMETER;
  } else {
    return EFI_SUCCESS;
  }
}

/**
  Create or update a String Token in a String Package.

  If *Reference == 0, a new String Token is created.

  @param This            A pointer to the EFI_HII_PROTOCOL instance.
  @param Language        Pointer to a NULL-terminated string containing a single ISO 639-2 language
                         identifier, indicating the language to print. A string consisting of
                         all spaces indicates that the string is applicable to all languages.
  @param Handle          The handle of the language pack to which the string is to be added.
  @param Token           The string token assigned to the string.
  @param NewString       The string to be added.


  @retval EFI_SUCCESS             The string was effectively registered.
  @retval EFI_INVALID_PARAMETER   The Handle was unknown. The string is not created or updated in the
                                  the string package.
**/

EFI_STATUS
EFIAPI
HiiNewString (
  IN     EFI_HII_PROTOCOL           *This,
  IN     CHAR16                     *Language,
  IN     FRAMEWORK_EFI_HII_HANDLE   Handle,
  IN OUT STRING_REF                 *Reference,
  IN     CHAR16                     *NewString
  )
{
  EFI_STATUS                                Status;
  HII_THUNK_PRIVATE_DATA                    *Private;
  EFI_GUID                                  TagGuid;
  LIST_ENTRY                                *Link;
  HII_THUNK_CONTEXT                          *ThunkContext;
  HII_THUNK_CONTEXT                          *StringPackThunkContext;
  EFI_STRING_ID                             StringId;
  EFI_STRING_ID                             LastStringId;
  CHAR8                                     AsciiLanguage[ISO_639_2_ENTRY_SIZE + 1];
  CHAR16                                    LanguageCopy[ISO_639_2_ENTRY_SIZE + 1];
  CHAR8                                     *Rfc3066AsciiLanguage;

  LastStringId      = (EFI_STRING_ID) 0;
  StringId          = (EFI_STRING_ID) 0;
  Rfc3066AsciiLanguage = NULL;

  if (Language != NULL) {
    ZeroMem (AsciiLanguage, sizeof (AsciiLanguage));;
    ZeroMem (LanguageCopy, sizeof (LanguageCopy));
    CopyMem (LanguageCopy, Language, ISO_639_2_ENTRY_SIZE * sizeof (CHAR16));
    UnicodeStrToAsciiStr (LanguageCopy, AsciiLanguage);
    Rfc3066AsciiLanguage = ConvertIso639ToRfc3066 (AsciiLanguage);
    ASSERT (Rfc3066AsciiLanguage != NULL);
  }

  Private = HII_THUNK_PRIVATE_DATA_FROM_THIS(This);

  StringPackThunkContext = FwHiiHandleToThunkContext (Private, Handle);
  if (StringPackThunkContext == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (StringPackThunkContext->SharingStringPack) {
    Status = GetTagGuidByFwHiiHandle (Private, Handle, &TagGuid);
    ASSERT_EFI_ERROR (Status);

    Link = GetFirstNode (&Private->ThunkContextListHead);
    while (!IsNull (&Private->ThunkContextListHead, Link)) {
      ThunkContext = HII_THUNK_CONTEXT_FROM_LINK (Link);

      if (CompareGuid (&TagGuid, &ThunkContext->TagGuid)) {
        if (ThunkContext->SharingStringPack) {
          StringId = *Reference;
          Status = UpdateString (ThunkContext, Rfc3066AsciiLanguage, NewString, &StringId);
          if (EFI_ERROR (Status)) {
            break;
          }
          
          DEBUG_CODE_BEGIN ();
          if (*Reference == 0) {
            //
            // When creating new string token, make sure all created token is the same
            // for all string packages registered using FW HII interface.
            //
            if (LastStringId == (EFI_STRING_ID) 0) {
              LastStringId = StringId;
            } else {
              if (LastStringId != StringId) {
                ASSERT(FALSE);
              }
            }
          }
          DEBUG_CODE_END ();
          
        }
      }

      Link = GetNextNode (&Private->ThunkContextListHead, Link);
    }
  } else {
    StringId = *Reference;
    Status = UpdateString (StringPackThunkContext, Rfc3066AsciiLanguage, NewString, &StringId);
  }

  if (!EFI_ERROR (Status)) {
    if (*Reference == 0) {
      *Reference = StringId;
    }
  } else {
    //
    // Only EFI_INVALID_PARAMETER is defined in HII 0.92 specification.
    //
    Status = EFI_INVALID_PARAMETER;
  }
  
  return Status;
}

/**
  This function removes any new strings that were added after the initial string export for this handle.
  UEFI HII String Protocol does not have Reset String function. This function perform nothing.

  @param This            A pointer to the EFI_HII_PROTOCOL instance.
  @param Handle          The HII handle on which the string resides.

  @retval EFI_SUCCESS    This function is a NOP and always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
HiiResetStrings (
  IN     EFI_HII_PROTOCOL   *This,
  IN     FRAMEWORK_EFI_HII_HANDLE      Handle
  )
{
  return EFI_SUCCESS;
}

/**
 This function extracts a string from a package already registered with the EFI HII database.

  @param This            A pointer to the EFI_HII_PROTOCOL instance.
  @param Handle          The HII handle on which the string resides.
  @param Token           The string token assigned to the string.
  @param Raw             If TRUE, the string is returned unedited in the internal storage format described
                         above. If false, the string returned is edited by replacing <cr> with <space>
                         and by removing special characters such as the <wide> prefix.
  @param LanguageString  Pointer to a NULL-terminated string containing a single ISO 639-2 language
                         identifier, indicating the language to print. If the LanguageString is empty (starts
                         with a NULL), the default system language will be used to determine the language.
  @param BufferLength    Length of the StringBuffer. If the status reports that the buffer width is too
                         small, this parameter is filled with the length of the buffer needed.
  @param StringBuffer    The buffer designed to receive the characters in the string. Type EFI_STRING is
                         defined in String.

  @retval EFI_INVALID_PARAMETER If input parameter is invalid.
  @retval EFI_BUFFER_TOO_SMALL  If the *BufferLength is too small.
  @retval EFI_SUCCESS           Operation is successful.

**/
EFI_STATUS
EFIAPI
HiiThunkGetString (
  IN     EFI_HII_PROTOCOL           *This,
  IN     FRAMEWORK_EFI_HII_HANDLE   Handle,
  IN     STRING_REF                 Token,
  IN     BOOLEAN                    Raw,
  IN     CHAR16                     *LanguageString,
  IN OUT UINTN                      *BufferLengthTemp,
  OUT    EFI_STRING                 StringBuffer
  )
{
  HII_THUNK_PRIVATE_DATA                *Private;
  CHAR8                                 *Iso639AsciiLanguage;
  CHAR8                                 *Rfc3066AsciiLanguage;
  CHAR8                                 *SupportedLanguages;
  CHAR8                                 *PlatformLanguage;
  CHAR8                                 *BestLanguage;
  EFI_HII_HANDLE                        UefiHiiHandle;
  EFI_STATUS                            Status;

  Private = HII_THUNK_PRIVATE_DATA_FROM_THIS(This);

  Iso639AsciiLanguage = NULL;
  Rfc3066AsciiLanguage = NULL;

  if (LanguageString != NULL) {
    Iso639AsciiLanguage = AllocateZeroPool (StrLen (LanguageString) + 1);
    if (Iso639AsciiLanguage == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    UnicodeStrToAsciiStr  (LanguageString, Iso639AsciiLanguage);

    //
    // Caller of Framework HII Interface uses the Language Identification String defined 
    // in Iso639. So map it to the Language Identifier defined in RFC3066.
    //
    Rfc3066AsciiLanguage = ConvertIso639ToRfc3066 (Iso639AsciiLanguage);

    //
    // If Rfc3066AsciiLanguage is NULL, more language mapping must be added to 
    // Iso639ToRfc3066Map.
    //
    ASSERT (Rfc3066AsciiLanguage != NULL);
    
  }

  UefiHiiHandle = FwHiiHandleToUefiHiiHandle (Private, Handle);
  if (UefiHiiHandle == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  if (Rfc3066AsciiLanguage == NULL) {
    //
    // Get the languages that the package specified by HiiHandle supports
    //
    SupportedLanguages = HiiGetSupportedLanguages (UefiHiiHandle);
    if (SupportedLanguages == NULL) {
      goto Error2;
    }

    //
    // Get the current platform language setting
    //
    PlatformLanguage = GetEfiGlobalVariable (L"PlatformLang");
    if (PlatformLanguage == NULL) {
      goto Error1;
    }

    //
    // Get the best matching language from SupportedLanguages
    //
    BestLanguage = GetBestLanguage (
                     SupportedLanguages, 
                     FALSE,                // RFC 4646 mode
                     PlatformLanguage,     // Next highest priority
                     SupportedLanguages,   // Lowest priority 
                     NULL
                     );
    if (BestLanguage == NULL) {
      FreePool (PlatformLanguage);
Error1:
      FreePool (SupportedLanguages);
Error2:
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Status = mHiiStringProtocol->GetString (
                                 mHiiStringProtocol,
                                 BestLanguage,
                                 UefiHiiHandle,
                                 Token,
                                 StringBuffer,
                                 BufferLengthTemp,
                                 NULL
                                 );
    FreePool (BestLanguage);
  } else {
    Status = mHiiStringProtocol->GetString (
                                 mHiiStringProtocol,
                                 Rfc3066AsciiLanguage,
                                 UefiHiiHandle,
                                 Token,
                                 StringBuffer,
                                 BufferLengthTemp,
                                 NULL
                                 );
  }

Done:
	if (Iso639AsciiLanguage != NULL) {
    FreePool (Iso639AsciiLanguage);
  }
  
  return Status;
}

/**

  This function allows a program to extract a part of a string of not more than a given width.
  With repeated calls, this allows a calling program to extract "lines" of text that fit inside
  columns.  The effort of measuring the fit of strings inside columns is localized to this call.

  This is a deprecated API. No Framework HII module is calling it. This function will ASSERT and
  return EFI_UNSUPPORTED.

  @param This            A pointer to the EFI_HII_PROTOCOL instance.
  @param Handle          The HII handle on which the string resides.
  @param Token           The string token assigned to the string.
  @param Raw             If TRUE, the string is returned unedited in the internal storage format described
                         above. If false, the string returned is edited by replacing <cr> with <space>
                         and by removing special characters such as the <wide> prefix.
  @param LanguageString  Pointer to a NULL-terminated string containing a single ISO 639-2 language
                         identifier, indicating the language to print. If the LanguageString is empty (starts
                         with a NULL), the default system language will be used to determine the language.
  @param BufferLength    Length of the StringBuffer. If the status reports that the buffer width is too
                         small, this parameter is filled with the length of the buffer needed.
  @param StringBuffer    The buffer designed to receive the characters in the string. Type EFI_STRING is
                         defined in String.

  @retval EFI_UNSUPPORTED.
**/
EFI_STATUS
EFIAPI
HiiGetLine (
  IN     EFI_HII_PROTOCOL           *This,
  IN     FRAMEWORK_EFI_HII_HANDLE   Handle,
  IN     STRING_REF                 Token,
  IN OUT UINT16                     *Index,
  IN     UINT16                     LineWidth,
  IN     CHAR16                     *LanguageString,
  IN OUT UINT16                     *BufferLength,
  OUT    EFI_STRING                 StringBuffer
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}


