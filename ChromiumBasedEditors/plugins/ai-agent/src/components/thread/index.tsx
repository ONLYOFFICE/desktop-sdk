import { ThreadPrimitive } from "@assistant-ui/react";
import { motion } from "framer-motion";

import { ThreadWelcome } from "./sub-components/Welcome";
import { UserMessage } from "./sub-components/UserMessage";
import { EditComposer } from "./sub-components/ComposerEdit";
import { AssistantMessage } from "./sub-components/AssistantMessage";
import { Composer } from "./sub-components/Composer";

const Thread = () => {
  return (
    <ThreadPrimitive.Root
      className="bg-background flex h-[calc(100dvh-64px)] max-h-[calc(100dvh-64px)] flex-col"
      style={{
        ["--thread-max-width" as string]: "900px",
        ["--thread-padding-x" as string]: "1rem",
      }}
    >
      <ThreadPrimitive.Viewport className="relative flex min-w-0 flex-1 flex-col gap-6 overflow-y-auto">
        <ThreadWelcome />

        <ThreadPrimitive.Messages
          components={{
            UserMessage,
            EditComposer,
            AssistantMessage,
          }}
        />

        <ThreadPrimitive.If empty={false}>
          <motion.div className="min-h-6 min-w-6 shrink-0" />
        </ThreadPrimitive.If>
      </ThreadPrimitive.Viewport>

      <Composer />
    </ThreadPrimitive.Root>
  );
};

export { Thread };
