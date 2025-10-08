import { ThreadPrimitive } from "@assistant-ui/react";
import { motion } from "framer-motion";
import { useTranslation } from "react-i18next";

import { Loader } from "@/components/loader";
import useMessageStore from "@/store/useMessageStore";

import { ThreadWelcome } from "./sub-components/Welcome";
import { UserMessage } from "./sub-components/UserMessage";
import { AssistantMessage } from "./sub-components/AssistantMessage";
import { Composer } from "./sub-components/Composer";

import "./Thread.css";

const Thread = () => {
  const { isStreamRunning, isRequestRunning } = useMessageStore();
  const { t } = useTranslation();
  return (
    <ThreadPrimitive.Root
      className="flex h-full flex-col"
      style={{
        ["--thread-max-width" as string]: "687px",
        ["--thread-padding-x" as string]: "1rem",
      }}
    >
      <ThreadPrimitive.If empty>
        <ThreadPrimitive.Viewport className="relative flex min-w-0 flex-1 flex-col overflow-y-auto justify-center items-center">
          <div className="flex flex-col gap-[24px] w-full">
            <ThreadWelcome />
            <Composer />
          </div>
        </ThreadPrimitive.Viewport>
      </ThreadPrimitive.If>

      <ThreadPrimitive.If empty={false}>
        <ThreadPrimitive.Viewport className="relative flex min-w-0 flex-1 flex-col gap-6 overflow-y-auto">
          <ThreadPrimitive.Messages
            components={{
              UserMessage,
              AssistantMessage,
            }}
          />

          {!isRequestRunning && isStreamRunning ? (
            <div className="flex items-center justify-center">
              <div className="flex items-center gap-[8px] w-[var(--thread-max-width)] max-w-[var(--thread-max-width)] ps-[var(--thread-padding-x)]">
                <Loader />
                <p className="text-[14px] leading-[20px] font-normal text-[var(--chat-message-analyze-color)]">
                  {t("Analyzing")}
                </p>
              </div>
            </div>
          ) : null}

          <motion.div className="min-h-6 min-w-6 shrink-0" />
        </ThreadPrimitive.Viewport>

        <Composer />
      </ThreadPrimitive.If>
    </ThreadPrimitive.Root>
  );
};

export default Thread;
