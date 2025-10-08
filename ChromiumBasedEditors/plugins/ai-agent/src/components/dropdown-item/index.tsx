import { useRef, useState } from "react";
import { Item, Separator } from "@radix-ui/react-dropdown-menu";
import { ReactSVG } from "react-svg";

import { cn } from "@/lib/utils";

import ArrowRightIconUrl from "@/assets/arrow.right.svg?url";
import CheckedIconUrl from "@/assets/checked.svg?url";

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
  subMenu,
  checked,
}: DropDownItemProps) => {
  const [isSubMenuOpen, setIsSubMenuOpen] = useState(false);

  const itemRef = useRef<HTMLDivElement | null>(null);
  const submenuRef = useRef<HTMLDivElement | null>(null);

  if (isSeparator)
    return (
      <Separator className="h-px w-full bg-[var(--drop-down-menu-separator-color)] my-[4px]" />
    );

  const onSelect = (e: Event) => {
    if (withToggle) {
      e.stopPropagation();
      e.preventDefault();
    }
    onClick(e);
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
        "flex items-center justify-between gap-[16px] min-w-0 w-full max-w-full min-h-[32px] h-[32px] px-[12px] select-none cursor-pointer",
        "outline-none focus:outline-none focus-visible:outline-none ring-0 focus:ring-0 focus-visible:ring-0 border-0",
        "hover:bg-[var(--drop-down-menu-item-hover-color)] hover:text-[var(--drop-down-menu-item-hover-color)]",
        "data-[highlighted]:bg-[var(--drop-down-menu-item-active-color)] data-[highlighted]:text-[var(--drop-down-menu-item-active-color)]",
        "data-[disabled]:opacity-50 data-[disabled]:pointer-events-none",
        isActive
          ? "bg-[var(--drop-down-menu-item-active-color)] text-[var(--drop-down-menu-item-active-color)]"
          : ""
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
        <span className="flex-1 basis-0 min-w-0 truncate font-normal text-[14px] leading-[20px] text-[var(--drop-down-menu-item-color)]">
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
          contentClassName="ms-[12px] mt-[-15px]"
          containerRef={itemRef.current}
          dropdownRef={submenuRef}
        />
      ) : null}
      {checked ? <ReactSVG src={CheckedIconUrl} /> : null}
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
