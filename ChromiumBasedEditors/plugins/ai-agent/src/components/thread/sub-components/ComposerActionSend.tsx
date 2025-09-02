import { ComposerPrimitive, ThreadPrimitive } from "@assistant-ui/react";

import ArrowTopIconUrl from "@/assets/arrow.top.svg?url";
import StopIconUrl from "@/assets/stop.svg?url";

import { IconButton } from "@/components/icon-button";
import useMessageStore from "@/store/useMessageStore";

const BUTTON_STYLES =
  "rounded-[4px] cursor-pointer disabled:cursor-not-allowed flex items-center justify-center bg-[var(--primary-button-background-color)] hover:bg-[var(--primary-button-background-hover-color)] active:bg-[var(--primary-button-background-pressed-color)] disabled:opacity-[var(--primary-button-background-disabled-opacity)] disabled:hover:bg-[var(--primary-button-background-color)]";

const ComposerActionSend = () => {
  const { isStreamRunning } = useMessageStore();

  return isStreamRunning ? (
    <ComposerPrimitive.Cancel asChild>
      <IconButton
        iconName={StopIconUrl}
        size={24}
        color="var(--primary-button-color)"
        className={BUTTON_STYLES}
      />
    </ComposerPrimitive.Cancel>
  ) : (
    <ThreadPrimitive.If running={false}>
      <ComposerPrimitive.Send asChild>
        <IconButton
          iconName={ArrowTopIconUrl}
          size={24}
          color="var(--primary-button-color)"
          className={BUTTON_STYLES}
        />
      </ComposerPrimitive.Send>
    </ThreadPrimitive.If>
  );
};

export { ComposerActionSend };
