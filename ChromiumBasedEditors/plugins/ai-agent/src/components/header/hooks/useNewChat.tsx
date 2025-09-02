import { Plus } from "lucide-react";

import useThreadsStore from "@/store/useThreadsStore";

import { Button } from "../../button";

export const useNewChat = () => {
  const { onSwitchToNewThread } = useThreadsStore();

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
