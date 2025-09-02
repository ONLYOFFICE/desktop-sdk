import React from "react";

import AttachmentIconUrl from "@/assets/attachment.svg?url";

import { IconButton } from "../../icon-button";
import { DropdownMenu } from "../../dropdown";
import type { DropDownItemProps } from "../../dropdown-item/DropDownItem.types";

const ComposerActionAttachment = () => {
  const [isOpen, setIsOpen] = React.useState(false);

  const onOpenChange = (open: boolean) => {
    setIsOpen(open);
  };

  const trigger = (
    <IconButton
      iconName={AttachmentIconUrl}
      size={24}
      className="cursor-pointer rounded-[4px] bg-[var(--attachment-trigger-background-color)] hover:bg-[var(--attachment-trigger-background-hover-color)] active:bg-[var(--attachment-trigger-background-active-color)] outline-none"
      style={{
        backgroundColor: isOpen
          ? "var(--attachment-trigger-background-active-color)"
          : "",
      }}
      color="var(--attachment-trigger-icon-color)"
      isStroke
    />
  );

  const items: DropDownItemProps[] = [
    { text: "Add local file...", onClick: () => {} },
    { text: "", onClick: () => {}, isSeparator: true },
    { text: "Recent files", onClick: () => {}, subMenu: [] },
  ];

  return (
    <DropdownMenu trigger={trigger} items={items} onOpenChange={onOpenChange} />
  );
};

export { ComposerActionAttachment };
