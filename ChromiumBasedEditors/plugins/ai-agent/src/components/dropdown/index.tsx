import * as DropdownMenu from "@radix-ui/react-dropdown-menu";

import { cn } from "@/lib/utils";

import { DropDownItem } from "../dropdown-item";

import type { DropdownMenuProps } from "./DropDown.types";

const DropdownMenuComponent = ({
  trigger,
  items,
  side,
  align,
  sideOffset,
  contentClassName,
  maxWidth,
  open,
  containerRef,
  dropdownRef,
  onOpenChange,
}: DropdownMenuProps) => {
  const portalContainer = document.getElementById("app");

  return (
    <DropdownMenu.Root open={open} onOpenChange={onOpenChange}>
      <DropdownMenu.Trigger asChild>{trigger}</DropdownMenu.Trigger>
      <DropdownMenu.Portal container={containerRef ?? portalContainer}>
        <DropdownMenu.Content
          ref={dropdownRef}
          side={side ?? "bottom"}
          align={align ?? "start"}
          sideOffset={sideOffset ?? 6}
          data-side={side}
          data-align={align}
          data-orientation="vertical"
          className={cn(
            "z-50 w-fit border border-[var(--dropdown-border-color)] rounded-[8px] shadow-[var(--dropdown-box-shadow)] bg-[var(--dropdown-background-color)] py-[4px] max-h-56 overflow-y-auto",
            contentClassName
          )}
          style={maxWidth ? { maxWidth } : undefined}
        >
          {items.map((item) => (
            <DropDownItem key={item.text} {...item} />
          ))}
        </DropdownMenu.Content>
      </DropdownMenu.Portal>
    </DropdownMenu.Root>
  );
};

export { DropdownMenuComponent as DropdownMenu };
