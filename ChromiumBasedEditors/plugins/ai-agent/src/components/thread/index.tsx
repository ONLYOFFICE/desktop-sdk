import { ThreadPrimitive } from "@assistant-ui/react";
import { motion } from "framer-motion";

import { ThreadWelcome } from "./sub-components/Welcome";
import { UserMessage } from "./sub-components/UserMessage";
// import { EditComposer } from "./sub-components/ComposerEdit";
import { AssistantMessage } from "./sub-components/AssistantMessage";
import { Composer } from "./sub-components/Composer";

import "./Thread.css";

const Thread = () => {
  return (
    <ThreadPrimitive.Root
      className="bg-background flex h-[calc(100dvh-64px)] max-h-[calc(100dvh-64px)] flex-col"
      style={{
        ["--thread-max-width" as string]: "900px",
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
              // EditComposer,
              AssistantMessage,
            }}
          />

          <motion.div className="min-h-6 min-w-6 shrink-0" />
        </ThreadPrimitive.Viewport>

        <Composer />
      </ThreadPrimitive.If>
    </ThreadPrimitive.Root>
  );
};

export { Thread };
