import { useRef, useState } from "react";
import { Item, Separator } from "@radix-ui/react-dropdown-menu";
import { EllipsisVerticalIcon } from "lucide-react";

import { cn } from "@/lib/utils";

import ArrowRightIconUrl from "@/assets/arrow.right.svg?url";

import { DropdownMenu } from "../dropdown";
import { IconButton } from "../icon-button";

import type { DropDownItemProps } from "./DropDownItem.types";
import { ToggleButton } from "../toggle-button";

const DropDownItem = ({
  text,
  icon,
  iconSize = 16,
  iconColor,
  onClick,
  isActive,
  isSeparator,
  withToggle,
  toggleChecked,
  onToggleChange,
  actionsMenu,
  subMenu,
}: DropDownItemProps) => {
  const [isSubMenuOpen, setIsSubMenuOpen] = useState(false);

  const itemRef = useRef<HTMLDivElement | null>(null);
  const submenuRef = useRef<HTMLDivElement | null>(null);

  if (isSeparator)
    return (
      <Separator className="h-px w-full bg-[var(--dropdown-item-separator-color)] my-[4px]" />
    );

  const onSelect = (e: Event) => {
    if (withToggle) {
      e.stopPropagation();
      e.preventDefault();
    }
    onClick();
  };

  const handleMouseMove = (e: MouseEvent) => {
    if (!itemRef.current || !submenuRef.current) return;

    const mouseX = e.clientX;
    const mouseY = e.clientY;

    const itemRect = itemRef.current.getBoundingClientRect();
    const submenuRect = submenuRef.current?.getBoundingClientRect();

    const isInsideItem =
      mouseX >= itemRect.left &&
      mouseX <= itemRect.right &&
      mouseY >= itemRect.top &&
      mouseY <= itemRect.bottom;

    const isInsideSubmenu =
      submenuRect &&
      mouseX >= submenuRect.left &&
      mouseX <= submenuRect.right &&
      mouseY >= submenuRect.top &&
      mouseY <= submenuRect.bottom;

    if (!isInsideItem && !isInsideSubmenu) {
      setIsSubMenuOpen(false);
      window.removeEventListener("mousemove", handleMouseMove);
    }
  };

  const handleMouseEnter = () => {
    if (isSubMenuOpen || !subMenu) return;

    setIsSubMenuOpen(true);
    window.addEventListener("mousemove", handleMouseMove);
  };

  return (
    <Item
      className={cn(
        "dropdown-menu-item",
        "flex items-center justify-between gap-[16px] min-w-0 w-full max-w-full h-[32px] px-[12px] select-none cursor-pointer",
        "outline-none focus:outline-none focus-visible:outline-none ring-0 focus:ring-0 focus-visible:ring-0 border-0",
        "hover:bg-accent hover:text-accent-foreground",
        "data-[highlighted]:bg-accent data-[highlighted]:text-accent-foreground",
        "data-[disabled]:opacity-50 data-[disabled]:pointer-events-none",
        isActive ? "bg-accent text-accent-foreground" : ""
      )}
      onSelect={onSelect}
      onMouseEnter={handleMouseEnter}
      ref={itemRef}
    >
      <div className="flex items-center gap-[8px]">
        {icon && typeof icon === "string" ? (
          <IconButton iconName={icon} size={iconSize} color={iconColor} />
        ) : (
          icon ?? null
        )}
        <span className="flex-1 basis-0 min-w-0 truncate font-normal text-[12px] leading-[20px] text-[var(--dropdown-item-text-color)]">
          {text}
        </span>
      </div>
      {actionsMenu ? (
        <DropdownMenu
          trigger={<EllipsisVerticalIcon className="shrink-0" />}
          items={actionsMenu}
        />
      ) : null}
      {subMenu ? (
        <DropdownMenu
          trigger={<IconButton iconName={ArrowRightIconUrl} size={12} />}
          items={subMenu}
          side="right"
          align="start"
          sideOffset={0}
          open={isSubMenuOpen}
          contentClassName="ms-[12px] mt-[-15px]"
          containerRef={itemRef.current}
          dropdownRef={submenuRef}
        />
      ) : null}
      {withToggle && onToggleChange ? (
        <div onClick={(e) => e.stopPropagation()}>
          <ToggleButton
            checked={toggleChecked ?? false}
            onCheckedChange={onToggleChange}
            size="small"
          />
        </div>
      ) : null}
    </Item>
  );
};

export { DropDownItem };
