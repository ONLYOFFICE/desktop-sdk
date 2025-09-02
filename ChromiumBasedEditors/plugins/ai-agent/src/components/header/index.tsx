import { useThreads } from "./hooks/useThreads";
import { useNewChat } from "./hooks/useNewChat";
import { useSelectModel } from "./hooks/useSelectModel";

const Header = () => {
  const { threadsComponent } = useThreads();

  const { newChatComponent } = useNewChat();

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
