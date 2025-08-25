import { useState, useEffect } from "react";

import type { AppendMessage, ThreadMessageLike } from "@assistant-ui/react";

import { provider, type SendMessageReturnType } from "@/providers";
import {
  createMessage,
  readMessages,
  updateMessage,
} from "@/database/messages";

import useTools from "./useTools";

type UseMessagesProps = {
  threadId: string;
  isReady: boolean;
  insertThread: (title: string) => void;
  insertNewMessageToThread: () => void;
};

const useMessages = ({
  threadId,
  isReady,
  insertThread,
  insertNewMessageToThread,
}: UseMessagesProps) => {
  const [messages, setMessages] = useState<ThreadMessageLike[]>([]);

  const { tools, callTools } = useTools({
    isReady,
  });

  useEffect(() => {
    if (!isReady) return;

    if (!provider) return;

    if (tools.length === 0) return;

    provider.setTools(tools);
  }, [isReady, tools]);

  useEffect(() => {
    if (!isReady) return;

    readMessages(threadId).then((messages) => {
      if (messages?.length === 0) {
        setMessages([]);
      }
      setMessages(messages);
    });
  }, [threadId, isReady]);

  const convertMessage = (message: ThreadMessageLike) => {
    return message;
  };

  const handleStream = async (
    stream: SendMessageReturnType,
    afterToolCall?: boolean,
    messageUIDProp?: string
  ) => {
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

            setMessages((currentMessages) => [
              ...currentMessages.slice(0, -1),
              lastMessage,
            ]);

            if (!provider) return;

            const streamAfterToolCall =
              provider.sendMessageAfterToolCall(lastMessage);

            handleStream(streamAfterToolCall, true, messageUID);
          }
        }

        return;
      }

      if (!initedMessage) {
        setMessages((currentMessages) => [...currentMessages, message]);
        createMessage(threadId, messageUID, message);
        initedMessage = true;
      } else {
        updateMessage(messageUID, message);
        setMessages((currentMessages) => [
          ...currentMessages.slice(0, -1),
          message,
        ]);
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

    setMessages((currentMessages) => [...currentMessages, userMessage]);

    const stream = await provider.sendMessage([userMessage]);

    handleStream(stream);
  };

  return { messages, setMessages, convertMessage, onNew };
};

export default useMessages;
