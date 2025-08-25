import { Item, Separator } from "@radix-ui/react-dropdown-menu";
import { EllipsisVerticalIcon } from "lucide-react";

import { cn } from "../../lib/utils";

import type { DropDownItemProps } from "./DropDownItem.types";
import { DropdownMenu } from "../dropdown";

const DropDownItem = ({
  text,
  icon,
  onClick,
  isActive,
  isSeparator,
  actionsMenu,
}: DropDownItemProps) => {
  if (isSeparator) return <Separator className="h-px w-full bg-accent my-1" />;

  return (
    <Item
      className={cn(
        "flex items-center justify-between gap-[16px] min-w-0 w-full max-w-full px-3 py-1 rounded-sm select-none cursor-pointer",
        "outline-none focus:outline-none focus-visible:outline-none ring-0 focus:ring-0 focus-visible:ring-0 border-0",
        "hover:bg-accent hover:text-accent-foreground",
        "data-[highlighted]:bg-accent data-[highlighted]:text-accent-foreground",
        "data-[disabled]:opacity-50 data-[disabled]:pointer-events-none",
        isActive ? "bg-accent text-accent-foreground" : ""
      )}
      onSelect={onClick}
    >
      <div className="flex items-center gap-[8px]">
        {icon ? icon : null}
        <p className="flex-1 basis-0 min-w-0 truncate">{text}</p>
      </div>
      {actionsMenu ? (
        <DropdownMenu
          trigger={<EllipsisVerticalIcon className="shrink-0" />}
          items={actionsMenu}
        />
      ) : null}
    </Item>
  );
};

export { DropDownItem };
