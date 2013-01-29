/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EmbedLiteAppListener_H_
#define EmbedLiteAppListener_H_

#include "nsWeakReference.h"
#include "nsIObserver.h"
#include "nsIEmbedAppService.h"
#include "nsClassHashtable.h"
#include <string>
#include <map>

class EmbedLiteAppService : public nsIObserver,
                            public nsIEmbedAppService
{
public:
    EmbedLiteAppService();
    virtual ~EmbedLiteAppService();

    NS_DECL_ISUPPORTS
    NS_DECL_NSIOBSERVER
    NS_DECL_NSIEMBEDAPPSERVICE

    void RegisterView(uint32_t aId);
    void UnregisterView(uint32_t aId);
    void HandleAsyncMessage(const char* aMessage, const nsString& aData);

private:
    std::map<uint64_t, uint32_t> mIDMap;
    nsClassHashtable<nsCStringHashKey,
                     nsTArray<nsCOMPtr<nsIEmbedMessageListener> > > mMessageListeners;
};

#define NS_EMBED_LITE_APP_CONTRACTID "@mozilla.org/embedlite-app-service;1"
#define NS_EMBED_LITE_APP_SERVICE_CLASSNAME "EmbedLiteApp Component"
#define NS_EMBED_LITE_APP_SERVICE_CID NS_IEMBEDAPPSERVICE_IID

#endif /*EmbedLiteAppService_H_*/

