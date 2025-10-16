import React from "react";
import { ReactSVG } from "react-svg";

import { cn } from "@/lib/utils";

import ArrowBottomSvgUrl from "@/assets/arrow.bottom.svg?url";

import { DropdownMenu } from "../dropdown";
import type { DropDownItemProps } from "../dropdown-item/DropDownItem.types";

type ComboBoxProps = {
  placeholder?: string;
  value?: string;
  className?: string;
  isError?: boolean;
  withoutBg?: boolean;
  items: DropDownItemProps[];
};

const ComboBox = ({
  placeholder,
  value,
  className,
  isError,
  withoutBg,
  items,
}: ComboBoxProps) => {
  const [isOpen, setIsOpen] = React.useState(false);

  const containerRef = React.useRef<HTMLDivElement>(null);
  return (
    <DropdownMenu
      onOpenChange={(value: boolean) => setIsOpen(value)}
      trigger={
        <div
          ref={containerRef}
          className={cn(
            "h-[32px] rounded-[4px] ps-[12px] pe-[12px] box-border",
            "cursor-pointer flex items-center justify-between",
            "text-[var(--input-color)] text-[14px] leading-[16px]",
            isOpen
              ? "border bg-[var(--input-background-color)] border-[var(--input-active-border-color)]"
              : withoutBg
              ? "hover:bg-[var(--input-hover-background-color)] hover:border-[var(--input-hover-border-color)]"
              : "border bg-[var(--input-background-color)] border-[var(--input-border-color)] hover:bg-[var(--input-hover-background-color)] hover:border-[var(--input-hover-border-color)]",
            className,
            items.length === 0 ? "cursor-not-allowed pointer-events-none opacity-50" : ""
          )}
          style={{
            borderColor: isError ? "var(--border-error)" : undefined,
          }}
        >
          <span
            className={cn(
              value
                ? "text-[var(--input-color)]"
                : "text-[var(--input-placeholder-color)]"
            )}
          >
            {value || placeholder}
          </span>
          <ReactSVG
            src={ArrowBottomSvgUrl}
            className={cn(
              "w-[15px] h-[24px] flex items-center justify-center transition-transform",
              isOpen ? "rotate-180" : ""
            )}
          />
        </div>
      }
      align="start"
      side="bottom"
      containerRef={containerRef.current}
      matchTriggerWidth={true}
      items={items}
    />
  );
};

export { ComboBox };
