import { useThreads, type UseThreadsProps } from "./hooks/useThreads";
import { useNewChat, type UseNewChatProps } from "./hooks/useNewChat";
import { useSelectModel } from "./hooks/useSelectModel";

type HeaderProps = UseThreadsProps & UseNewChatProps;

const Header = ({
  threadId,
  threads,
  onSwitchToNewThread,
  onSwitchToThread,
  onDelete,
}: HeaderProps) => {
  const { threadsComponent } = useThreads({
    threadId,
    threads,
    onSwitchToThread,
    onDelete,
  });

  const { newChatComponent } = useNewChat({
    onSwitchToNewThread,
  });

  const { useSelectModelComponent } = useSelectModel();

  return (
    <div className="flex items-center gap-[16px] px-[32px]  border-b border-border">
      {threadsComponent}

      {newChatComponent}

      {useSelectModelComponent}
    </div>
  );
};

export { Header };
