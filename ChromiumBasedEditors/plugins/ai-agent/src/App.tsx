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
import useRouter from "./store/useRouter";
import useProviders from "./store/useProviders";
import useServersStore from "./store/useServersStore";

import EmptyScreen from "./pages/empty-screen";
import Settings from "./pages/settings";
import Thread from "./pages/chat";

import { Layout } from "./components/layout";
import { ManageToolDialog } from "./components/manage-tool-dialog";

import useMessages from "./hooks/useMessages";
import useThread from "./hooks/useThreads";
import useModels from "./hooks/useModels";
import useServers from "./hooks/useServers";

import { chatDB, initChatDB } from "./database";

import "./i18n";

const App = () => {
  const [isReady, setIsReady] = useState(false);

  const [isManageToolOpen, setIsManageToolOpen] = useState(false);

  const { messages, stopMessage } = useMessageStore();
  const { providers, fetchProvidersModels } = useProviders();
  const { currentPage } = useRouter();
  const { manageToolData } = useServersStore();

  useThread({
    isReady,
  });

  useModels();

  useServers({
    isReady,
  });

  const { onNew, convertMessage, approveToolCall, denyToolCall } = useMessages({
    isReady,
  });

  useEffect(() => {
    if (providers.length) fetchProvidersModels();
  }, [providers, fetchProvidersModels]);

  useEffect(() => {
    if (manageToolData) setIsManageToolOpen(true);
  }, [manageToolData]);

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

  if (currentPage !== "settings" && !providers.length)
    return (
      <Layout>
        <EmptyScreen />
      </Layout>
    );

  return (
    <Layout>
      <AssistantRuntimeProvider runtime={runtime}>
        {currentPage === "settings" ? <Settings /> : <Thread />}
      </AssistantRuntimeProvider>
      {isManageToolOpen ? (
        <ManageToolDialog
          onAllow={approveToolCall}
          onDeny={denyToolCall}
          onClose={() => setIsManageToolOpen(false)}
        />
      ) : null}
    </Layout>
  );
};

export default App;
