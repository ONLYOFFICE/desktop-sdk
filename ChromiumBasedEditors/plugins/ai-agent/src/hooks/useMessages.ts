import { useEffect } from "react";

import type { AppendMessage, ThreadMessageLike } from "@assistant-ui/react";

import { provider, type SendMessageReturnType } from "@/providers";
import { createMessage, updateMessage } from "@/database/messages";
import useModelsStore from "@/store/useModelsStore";
import useMessageStore from "@/store/useMessageStore";
import useThreadsStore from "@/store/useThreadsStore";
import useServersStore from "@/store/useServersStore";

type UseMessagesProps = {
  isReady: boolean;
};

const useMessages = ({ isReady }: UseMessagesProps) => {
  const { currentModel } = useModelsStore();
  const {
    messages,
    setIsStreamRunning,
    addMessage,
    updateLastMessage,
    fetchPrevMessages,
    getCurrentProviderModel,
    setCurrentProvider,
    updateCurrentProvider,
  } = useMessageStore();
  const { threadId, insertThread, insertNewMessageToThread } =
    useThreadsStore();
  const { tools, callTools } = useServersStore();

  useEffect(() => {
    if (!currentModel) return;

    const currentProviderModel = getCurrentProviderModel();

    if (currentProviderModel === currentModel.id) return;

    if (currentModel.provider === provider.currentProviderType) {
      updateCurrentProvider(currentModel.id);

      return;
    }

    setCurrentProvider(currentModel.provider);
    updateCurrentProvider(currentModel.id, tools, messages);
  }, [
    tools,
    currentModel,
    messages,
    getCurrentProviderModel,
    updateCurrentProvider,
    setCurrentProvider,
  ]);

  useEffect(() => {
    if (!isReady) return;

    updateCurrentProvider(undefined, tools);
  }, [tools, isReady, updateCurrentProvider]);

  useEffect(() => {
    if (!isReady) return;

    fetchPrevMessages(threadId);
  }, [threadId, isReady, fetchPrevMessages]);

  const convertMessage = (message: ThreadMessageLike) => {
    return message;
  };

  const handleStream = async (
    stream: SendMessageReturnType,
    afterToolCall?: boolean,
    messageUIDProp?: string
  ) => {
    setIsStreamRunning(true);
    let initedMessage = afterToolCall ? true : false;
    const messageUID =
      afterToolCall && messageUIDProp ? messageUIDProp : crypto.randomUUID();

    for await (const message of stream) {
      if ("isEnd" in message) {
        const lastMessage = message.responseMessage;

        if (
          lastMessage?.role === "assistant" &&
          Array.isArray(lastMessage.content)
        ) {
          const toolCallIdx = lastMessage.content.findIndex(
            (c) => c.type === "tool-call" && !c.result
          );

          if (toolCallIdx !== -1) {
            const toolCall = lastMessage.content[toolCallIdx];

            const result = callTools(toolCall.toolName, toolCall.args);

            toolCall.result = result;

            lastMessage.content[toolCallIdx] = toolCall;

            updateLastMessage(lastMessage);

            if (!provider) return;

            const streamAfterToolCall =
              provider.sendMessageAfterToolCall(lastMessage);

            if (streamAfterToolCall)
              handleStream(streamAfterToolCall, true, messageUID);
          }
        }

        setIsStreamRunning(false);

        return;
      }

      if (!initedMessage) {
        addMessage(message);
        createMessage(threadId, messageUID, message);
        initedMessage = true;
      } else {
        updateMessage(messageUID, message);
        updateLastMessage(message);
      }
    }
  };

  const onNew = async (message: AppendMessage) => {
    if (!provider) return;
    if (message.content[0].type !== "text") return;

    const userMessage: ThreadMessageLike = {
      role: "user",
      content: [{ type: "text", text: message.content[0].text }],
      attachments: message.attachments,
    };

    if (messages.length === 0) {
      insertThread(message.content[0].text.substring(0, 25));
    } else {
      insertNewMessageToThread();
    }

    createMessage(threadId, crypto.randomUUID(), userMessage);

    addMessage(userMessage);

    const stream = provider.sendMessage([userMessage]);

    if (stream) handleStream(stream);
  };

  return {
    convertMessage,
    onNew,
    handleStream,
  };
};

export default useMessages;
