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
  matchTriggerWidth,
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
            "dropdown-menu z-50 border border-[var(--drop-down-menu-border-color)] rounded-[8px] shadow-[var(--drop-down-menu-shadow)] bg-[var(--drop-down-menu-background-color)] py-[8px] max-h-56 overflow-y-auto flex flex-col gap-[4px]",
            matchTriggerWidth
              ? "w-[var(--radix-dropdown-menu-trigger-width)]"
              : "w-fit",
            contentClassName
          )}
          style={maxWidth ? { maxWidth } : undefined}
        >
          {items.map((item, index) => (
            <DropDownItem key={item.id || item.text || `item-${index}`} {...item} />
          ))}
        </DropdownMenu.Content>
      </DropdownMenu.Portal>
    </DropdownMenu.Root>
  );
};

export { DropdownMenuComponent as DropdownMenu };
