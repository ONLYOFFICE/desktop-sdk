export type DropDownItemProps = {
  text: string;
  icon?: React.ReactNode;
  onClick: () => void;

  id?: string;
  isActive?: boolean;
  isSeparator?: boolean;
  actionsMenu?: DropDownItemProps[];
};
