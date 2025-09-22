import { useTranslation } from "react-i18next";

import { Button } from "@/components/button";

type WalletProps = {
  isActive: boolean;
};

const Wallet = ({ isActive }: WalletProps) => {
  const { t } = useTranslation();

  return (
    <Button className="max-w-[fit-content]" disabled={!isActive}>
      {t("RegisterConnectWallet")}
    </Button>
  );
};

export { Wallet };
