import { useRef, useState } from "react";
import { Item } from "@radix-ui/react-dropdown-menu";

import { cn } from "@/lib/utils";

import ArrowRightIconUrl from "@/assets/arrow.right.svg?url";
import CheckedIconUrl from "@/assets/checked.svg?url";

import { DropdownMenu } from "../dropdown";
import { IconButton } from "../icon-button";
import { Tooltip, TooltipTrigger, TooltipContent } from "../tooltip";

import type { DropDownItemProps } from "./DropDownItem.types";
import { ToggleButton } from "../toggle-button";

const DropDownItem = ({
  text,
  icon,
  iconSize = 16,
  onClick,
  isActive,
  isSeparator,
  withToggle,
  toggleChecked,
  onToggleChange,
  toggleDisabled,
  subMenu,
  checked,
  tooltipText,
  withSpace,
}: DropDownItemProps) => {
  const [isSubMenuOpen, setIsSubMenuOpen] = useState(false);

  const itemRef = useRef<HTMLDivElement | null>(null);
  const submenuRef = useRef<HTMLDivElement | null>(null);

  if (isSeparator)
    return (
      <div className="h-px min-h-[1px] w-full bg-[var(--drop-down-menu-separator-color)] my-[4px] flex-shrink-0" />
    );

  const onSelect = (e: Event) => {
    if (withToggle) {
      e.stopPropagation();
      e.preventDefault();
      return;
    }
    onClick(e);
  };

  const handleToggleClick = (e: React.MouseEvent) => {
    e.stopPropagation();
    e.preventDefault();

    if (toggleDisabled) return;

    onToggleChange?.(!toggleChecked);
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

  const itemContent = (
    <Item
      className={cn(
        "dropdown-menu-item",
        "flex items-center justify-between gap-[16px] min-w-0 w-full max-w-full min-h-[32px] h-[32px] px-[12px] select-none cursor-pointer",
        "outline-none focus:outline-none focus-visible:outline-none ring-0 focus:ring-0 focus-visible:ring-0 border-0",
        "hover:bg-[var(--drop-down-menu-item-hover-color)] hover:text-[var(--drop-down-menu-item-hover-color)]",
        // "data-[highlighted]:bg-[var(--drop-down-menu-item-active-color)] data-[highlighted]:text-[var(--drop-down-menu-item-active-color)]",
        "data-[disabled]:opacity-50 data-[disabled]:pointer-events-none",
        isActive || (subMenu && isSubMenuOpen)
          ? "bg-[var(--drop-down-menu-item-active-color)] text-[var(--drop-down-menu-item-active-color)]"
          : ""
      )}
      onSelect={withToggle ? (e) => e.preventDefault() : onSelect}
      onClick={withToggle ? handleToggleClick : undefined}
      onMouseEnter={handleMouseEnter}
      ref={itemRef}
    >
      <div className="flex items-center gap-[8px] min-w-0 flex-1">
        {icon && typeof icon === "string" ? (
          <IconButton iconName={icon} size={iconSize} disableHover />
        ) : (
          icon ?? null
        )}
        <span
          className={cn(
            "truncate font-normal text-[14px] leading-[20px] text-[var(--drop-down-menu-item-color)]",
            withSpace ? "ms-[28px]" : ""
          )}
        >
          {text}
        </span>
      </div>

      {subMenu ? (
        <DropdownMenu
          trigger={
            <IconButton
              iconName={ArrowRightIconUrl}
              size={12}
              insideElement
              isStroke
            />
          }
          items={subMenu}
          side="right"
          align="start"
          sideOffset={0}
          open={isSubMenuOpen}
          contentClassName="ms-[12px] mt-[-15px] max-w-[300px]"
          containerRef={itemRef.current}
          dropdownRef={submenuRef}
        />
      ) : null}
      {checked ? (
        <IconButton iconName={CheckedIconUrl} size={16} disableHover isStroke />
      ) : null}
      {withToggle && onToggleChange ? (
        <div onClick={(e) => e.stopPropagation()}>
          <ToggleButton
            checked={toggleChecked ?? false}
            onCheckedChange={onToggleChange}
            disabled={toggleDisabled}
          />
        </div>
      ) : null}
    </Item>
  );

  if (tooltipText) {
    return (
      <Tooltip>
        <TooltipTrigger asChild>{itemContent}</TooltipTrigger>
        <TooltipContent>{tooltipText}</TooltipContent>
      </Tooltip>
    );
  }

  return itemContent;
};

export { DropDownItem };
