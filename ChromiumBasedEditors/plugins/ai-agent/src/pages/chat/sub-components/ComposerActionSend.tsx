import { ComposerPrimitive, ThreadPrimitive } from "@assistant-ui/react";

import ArrowTopIconUrl from "@/assets/arrow.top.svg?url";
import StopIconUrl from "@/assets/stop.svg?url";

import useMessageStore from "@/store/useMessageStore";
import useModelsStore from "@/store/useModelsStore";
import useProviders from "@/store/useProviders";

import { IconButton } from "@/components/icon-button";

const BUTTON_STYLES =
  "rounded-[4px] cursor-pointer disabled:cursor-not-allowed flex items-center justify-center bg-[var(--chat-composer-action-send-background-color)] hover:enabled:bg-[var(--chat-composer-action-send-background-hover-color)] active:enabled:bg-[var(--chat-composer-action-send-background-pressed-color)] disabled:opacity-[0.5]";

const ComposerActionSend = () => {
  const { isStreamRunning } = useMessageStore();
  const { currentModel } = useModelsStore();
  const { currentProvider } = useProviders();

  return isStreamRunning ? (
    <ComposerPrimitive.Cancel asChild>
      <IconButton
        iconName={StopIconUrl}
        size={24}
        color="var(--chat-composer-action-send-color)"
        className={BUTTON_STYLES}
      />
    </ComposerPrimitive.Cancel>
  ) : (
    <ThreadPrimitive.If running={false}>
      <ComposerPrimitive.Send
        asChild
        disabled={!currentModel || !currentProvider}
      >
        <IconButton
          iconName={ArrowTopIconUrl}
          size={24}
          color="var(--chat-composer-action-send-color)"
          className={BUTTON_STYLES}
        />
      </ComposerPrimitive.Send>
    </ThreadPrimitive.If>
  );
};

export { ComposerActionSend };
