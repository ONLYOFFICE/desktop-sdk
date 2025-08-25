import { useEffect, useState } from "react";

import {
  AssistantRuntimeProvider,
  useExternalStoreRuntime,
  type ThreadMessageLike,
  CompositeAttachmentAdapter,
  SimpleImageAttachmentAdapter,
  SimpleTextAttachmentAdapter,
} from "@assistant-ui/react";

import { Thread } from "./components/thread";
import { Header } from "./components/header";

import useMessages from "./hooks/useMessages";
import useThread from "./hooks/useThreads";

import { chatDB, initChatDB } from "./database";

const App = () => {
  const [isReady, setIsReady] = useState(false);

  const {
    threads,
    threadId,
    onSwitchToNewThread,
    onSwitchToThread,
    onDeleteThread,
    insertThread,
    insertNewMessageToThread,
  } = useThread({
    isReady,
  });

  const { messages, setMessages, onNew, convertMessage } = useMessages({
    threadId,
    isReady,
    insertThread,
    insertNewMessageToThread,
  });

  useEffect(() => {
    initChatDB().then(() => setIsReady(true));

    return () => {
      chatDB.close();
    };
  }, []);

  const runtime = useExternalStoreRuntime<ThreadMessageLike>({
    messages,
    setMessages,
    onNew,
    convertMessage,
    adapters: {
      attachments: new CompositeAttachmentAdapter([
        new SimpleImageAttachmentAdapter(),
        new SimpleTextAttachmentAdapter(),
      ]),
    },
  });

  return (
    <div className="h-dvh max-h-screen">
      <AssistantRuntimeProvider runtime={runtime}>
        <div className="grid h-dvh grid-cols-1f grid-rows-[64px_1fr] gap-x-2 max-h-screen overflow-hidden">
          <Header
            threadId={threadId}
            threads={threads}
            onSwitchToNewThread={onSwitchToNewThread}
            onSwitchToThread={onSwitchToThread}
            onDelete={onDeleteThread}
          />
          <Thread />
        </div>
      </AssistantRuntimeProvider>
    </div>
  );
};

export default App;
