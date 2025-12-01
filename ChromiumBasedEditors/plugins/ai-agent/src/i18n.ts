import i18n from "i18next";
import { initReactI18next } from "react-i18next";

import en from "./translations/en.json";
import ru from "./translations/ru.json";
import cs from "./translations/cs.json";
import de from "./translations/de.json";
import es from "./translations/es.json";
import fr from "./translations/fr.json";
import it from "./translations/it.json";
import pl from "./translations/pl.json";
import ptBr from "./translations/pt-br.json";
import sk from "./translations/sk.json";

i18n
  .use(initReactI18next) // passes i18n down to react-i18next
  .init({
    resources: {
      en: {
        translation: en,
      },
      ru: {
        translation: ru,
      },
      "cs-CZ": {
        translation: cs,
      },
      de: {
        translation: de,
      },
      es: {
        translation: es,
      },
      fr: {
        translation: fr,
      },
      it: {
        translation: it,
      },
      pl: {
        translation: pl,
      },
      "pt-BR": {
        translation: ptBr,
      },
      "sl-SI": {
        translation: sk,
      },
    },
    fallbackLng: "en",

    interpolation: {
      escapeValue: false, // react already safes from xss => https://www.i18next.com/translation-function/interpolation#unescape
    },
  });
