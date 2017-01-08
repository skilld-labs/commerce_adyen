# Commerce Adyen

This module provides a [Drupal Commerce](https://www.drupal.org/project/commerce)
payment method to embed the payment services provided by 
[Adyen](https://www.adyen.com). It efficiently integrates payments from various
sources such as credit cards, bank transfers, PayPal and also mobile phones.

This modules only supports the 
[Adyen Hosted Payment Pages (HPP)](https://docs.adyen.com/developers/hpp-manual)
solution and not direct API integration. The module also supports the
[Adyen manual capture](https://docs.adyen.com/developers/api-manual#capture)
process.

## Versions

- **7.x-1.x** branch no longer be maintained.
- **7.x-2.x** actively maintained.

**Version 7.x-2.x is still in development and no guarantees are provided about
its functionality. Use in a production environment at your own risk.**

## Installation

- Enable the module.
- Clear cache.

## Configuration

To access the configuration UI you must first ensure that the **Payment UI**
(commerce_payment_ui) module is enabled. Then you can go to the following URL:

`admin/commerce/config/payment-methods`

From this page simply click the **Edit** link next to **Adyen** and you will be
taken to the config form. From here you can enter your Adyen credentials.
