/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _nsHTMLImageAccessible_H_
#define _nsHTMLImageAccessible_H_

#include "nsBaseWidgetAccessible.h"
#include "nsIAccessibleImage.h"

class nsGenericHTMLElement;

/* Accessible for supporting images
 * supports:
 * - gets name, role
 * - support basic state
 */
class nsHTMLImageAccessible : public nsLinkableAccessible,
                              public nsIAccessibleImage
{
public:
  nsHTMLImageAccessible(nsIContent* aContent, DocAccessible* aDoc);

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIAccessible
  NS_IMETHOD GetActionName(PRUint8 aIndex, nsAString& aName);
  NS_IMETHOD DoAction(PRUint8 index);

  // nsIAccessibleImage
  NS_DECL_NSIACCESSIBLEIMAGE

  // Accessible
  virtual nsresult GetNameInternal(nsAString& aName);
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();
  virtual nsresult GetAttributesInternal(nsIPersistentProperties *aAttributes);

  // ActionAccessible
  virtual PRUint8 ActionCount();

private:
  /**
   * Return whether the element has a longdesc URI.
   */
  bool HasLongDesc() const
  {
    nsCOMPtr<nsIURI> uri = GetLongDescURI();
    return uri;
  }

  /**
   * Return an URI for showlongdesc action if any.
   */
  already_AddRefed<nsIURI> GetLongDescURI() const;

  /**
   * Used by GetActionName and DoAction to ensure the index for opening the
   * longdesc URL is valid.
   * It is always assumed that the highest possible index opens the longdesc.
   * This doesn't check that there is actually a longdesc, just that the index
   * would be correct if there was one.
   *
   * @param aIndex  The 0-based index to be tested.
   *
   * @returns  true if index is valid for longdesc action.
   */
  inline bool IsLongDescIndex(PRUint8 aIndex);

};

////////////////////////////////////////////////////////////////////////////////
// Accessible downcasting method

inline nsHTMLImageAccessible*
Accessible::AsImage()
{
  return IsImage() ?
    static_cast<nsHTMLImageAccessible*>(this) : nsnull;
}

#endif

