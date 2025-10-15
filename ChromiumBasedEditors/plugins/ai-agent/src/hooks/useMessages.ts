import { useEffect } from "react";

import type {
  AppendMessage,
  FileMessagePart,
  ThreadMessageLike,
} from "@assistant-ui/react";

import { provider, type SendMessageReturnType } from "@/providers";
import { createMessage, updateMessage } from "@/database/messages";

import useMessageStore from "@/store/useMessageStore";
import useAttachmentsStore from "@/store/useAttachmentsStore";
import useThreadsStore from "@/store/useThreadsStore";
import useServersStore from "@/store/useServersStore";
import useProviders from "@/store/useProviders";
import useModelsStore from "@/store/useModelsStore";

type UseMessagesProps = {
  isReady: boolean;
};

const useMessages = ({ isReady }: UseMessagesProps) => {
  const {
    messages,
    setIsStreamRunning,
    setIsRequestRunning,
    addMessage,

    updateLastMessage,
    fetchPrevMessages,
  } = useMessageStore();
  const { threadId, insertThread, insertNewMessageToThread } =
    useThreadsStore();
  const {
    manageToolData,
    callTools,
    checkAllowAlways,
    setAllowAlways,
    setManageToolData,
  } = useServersStore();
  const { attachmentFiles, clearAttachmentFiles } = useAttachmentsStore();
  const { currentProvider } = useProviders();
  const { currentModel } = useModelsStore();

  useEffect(() => {
    if (!isReady) return;

    fetchPrevMessages(threadId);
    clearAttachmentFiles();
  }, [threadId, isReady, fetchPrevMessages, clearAttachmentFiles]);

  const convertMessage = (message: ThreadMessageLike) => {
    return message;
  };

  const approveToolCall = (allowAlways: boolean) => {
    if (!manageToolData) return;

    const toolCall = manageToolData?.message?.content[manageToolData.idx];

    if (
      !toolCall ||
      typeof toolCall !== "object" ||
      !("type" in toolCall) ||
      toolCall.type !== "tool-call"
    )
      return;

    const toolName = toolCall.toolName;

    const type = toolName.split("_")[0];

    if (allowAlways) {
      setAllowAlways(true, type);
    }

    handleToolCall(
      manageToolData.message,
      manageToolData.idx,
      manageToolData.messageUID,
      true,
      false
    );

    setManageToolData(undefined);
  };

  const denyToolCall = () => {
    if (!manageToolData) return;

    handleToolCall(
      manageToolData.message,
      manageToolData.idx,
      manageToolData.messageUID,
      false,
      true
    );

    setManageToolData(undefined);
  };

  const handleToolCall = async (
    msg: ThreadMessageLike,
    idx: number,
    messageUID: string,
    accept?: boolean,
    deny?: boolean
  ) => {
    const toolCall = msg.content[idx];

    if (
      !toolCall ||
      typeof toolCall !== "object" ||
      !("type" in toolCall) ||
      toolCall.type !== "tool-call"
    )
      return;

    const toolName = toolCall.toolName;

    const type = toolName.split("_")[0];

    if (checkAllowAlways(type) || accept || deny) {
      const result = deny
        ? "User deny tool call"
        : await callTools(
            toolCall.toolName,
            toolCall.args as Record<string, unknown>
          );

      // Create a new content array with the updated tool call
      const updatedContent = Array.isArray(msg.content)
        ? msg.content.map((item, index) =>
            index === idx ? { ...toolCall, result } : item
          )
        : msg.content;

      // Create a new message object with updated content
      const updatedMessage = { ...msg, content: updatedContent };

      updateLastMessage(updatedMessage);

      if (!provider) return;

      const streamAfterToolCall =
        provider.sendMessageAfterToolCall(updatedMessage);

      if (streamAfterToolCall) {
        handleStream(streamAfterToolCall, true, messageUID);
      }
    } else {
      setManageToolData({
        message: msg,
        idx,
        messageUID,
      });
    }
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

    if (messages)
      for await (const message of stream) {
        if ("isEnd" in message) {
          if (message.responseMessage.status?.type === "incomplete") {
            addMessage(message.responseMessage);

            setIsStreamRunning(false);
            setIsRequestRunning(false);

            return;
          }
          const lastMessage = message.responseMessage;

          if (
            lastMessage?.role === "assistant" &&
            Array.isArray(lastMessage.content)
          ) {
            const toolCallIdx = lastMessage.content.findIndex(
              (c) => c.type === "tool-call" && !c.result
            );

            if (toolCallIdx !== -1) {
              handleToolCall(lastMessage, toolCallIdx, messageUID);

              return;
            }
          }

          setIsStreamRunning(false);
          setIsRequestRunning(false);

          return;
        }

        if (!initedMessage) {
          if (!afterToolCall) setIsRequestRunning(true);
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
    if (!currentProvider || !currentModel) return;
    if (message.content[0].type !== "text") return;

    let fileContent: FileMessagePart[] = [];

    if (attachmentFiles.length > 0) {
      fileContent = attachmentFiles.map((file) => ({
        type: "file",
        mimeType: JSON.stringify({ path: file.path, type: file.type }),
        data: file.content,
      }));

      clearAttachmentFiles();
    }

    const content: ThreadMessageLike["content"] = [
      ...fileContent,
      { type: "text", text: message.content[0].text },
    ];

    const userMessage: ThreadMessageLike = {
      role: "user",
      content,
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
    approveToolCall,
    denyToolCall,
  };
};

export default useMessages;
