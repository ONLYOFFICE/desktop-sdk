import * as DropdownMenu from "@radix-ui/react-dropdown-menu";
import { cn } from "../../lib/utils";
import { DropDownItem } from "../dropdown-item";

import type { DropdownMenuProps } from "./DropDown.types";

const DropdownMenuComponent = ({
  trigger,
  items,
  side,
  align,
  contentClassName,
  maxWidth,
}: DropdownMenuProps) => {
  return (
    <DropdownMenu.Root>
      <DropdownMenu.Trigger asChild>
        <button
          type="button"
          className="cursor-pointer appearance-none outline-none outline-offset-0 focus:outline-none focus-visible:outline-none focus:outline-offset-0 ring-0 ring-offset-0 focus:ring-0 focus:ring-transparent focus-visible:ring-0 focus-visible:ring-transparent border-0 focus:border-transparent focus-visible:border-transparent bg-transparent p-0 m-0 shadow-none focus:shadow-none focus-visible:shadow-none"
        >
          {trigger}
        </button>
      </DropdownMenu.Trigger>
      <DropdownMenu.Portal>
        <DropdownMenu.Content
          side={side ?? "bottom"}
          align={align ?? "start"}
          data-side={side}
          data-align={align}
          data-orientation="vertical"
          sideOffset={4}
          className={cn(
            "z-50 w-fit border border-border rounded-md shadow-md bg-white py-[8px] max-h-56 overflow-y-auto",
            contentClassName
          )}
          style={maxWidth ? { maxWidth } : undefined}
        >
          {items.map((item, index) => (
            <DropDownItem
              key={index}
              onClick={item.onClick}
              text={item.text}
              icon={item.icon}
              actionsMenu={item.actionsMenu}
              isActive={item.isActive}
              isSeparator={item.isSeparator}
            />
          ))}
        </DropdownMenu.Content>
      </DropdownMenu.Portal>
    </DropdownMenu.Root>
  );
};

export { DropdownMenuComponent as DropdownMenu };
