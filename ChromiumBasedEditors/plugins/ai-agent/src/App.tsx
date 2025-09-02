import { useEffect, useState } from "react";

import {
  AssistantRuntimeProvider,
  useExternalStoreRuntime,
  CompositeAttachmentAdapter,
  SimpleImageAttachmentAdapter,
  SimpleTextAttachmentAdapter,
  type ThreadMessageLike,
} from "@assistant-ui/react";

import useMessageStore from "./store/useMessageStore";

import { Thread } from "./components/thread";
import { Header } from "./components/header";

import useMessages from "./hooks/useMessages";
import useThread from "./hooks/useThreads";
import useModels from "./hooks/useModels";
import useServers from "./hooks/useServers";

import { chatDB, initChatDB } from "./database";

const App = () => {
  const [isReady, setIsReady] = useState(false);

  const { messages, stopMessage } = useMessageStore();

  useThread({
    isReady,
  });

  useModels();

  useServers({
    isReady,
  });

  const { onNew, convertMessage } = useMessages({
    isReady,
  });

  useEffect(() => {
    initChatDB().then(() => setIsReady(true));

    return () => {
      chatDB.close();
    };
  }, []);

  const runtime = useExternalStoreRuntime<ThreadMessageLike>({
    messages,
    onNew,
    onCancel: async () => {
      stopMessage();
    },
    convertMessage,
    adapters: {
      attachments: new CompositeAttachmentAdapter([
        new SimpleImageAttachmentAdapter(),
        new SimpleTextAttachmentAdapter(),
      ]),
    },
  });

  return (
    <div id="app" className="h-dvh max-h-screen theme-light">
      <AssistantRuntimeProvider runtime={runtime}>
        <div className="grid h-dvh grid-cols-1f grid-rows-[64px_1fr] gap-x-2 max-h-screen overflow-hidden">
          <Header />
          <Thread />
        </div>
      </AssistantRuntimeProvider>
    </div>
  );
};

export default App;
