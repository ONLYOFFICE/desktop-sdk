import { Plus } from "lucide-react";
import { Button } from "../../button";

export type UseNewChatProps = {
  onSwitchToNewThread: () => void;
};

export const useNewChat = ({ onSwitchToNewThread }: UseNewChatProps) => {
  const newChatComponent = (
    <Button variant="outline" onClick={onSwitchToNewThread}>
      <Plus />
      Create new chat
    </Button>
  );
  return {
    newChatComponent,
  };
};
