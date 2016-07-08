<?php

/**
 * @file
 * Hook documentation for the PayPal WPS module.
 */


/**
 * Allows modules to alter the data array used to create a Adyen redirect
 * form prior to its form elements being created.
 *
 * @param &$data
 *   The data array used to create redirect form elements.
 * @param $order
 *   The full order object the redirect form is being generated for.
 *
 * @see commerce_adyen_pro_redirect_form()
 */
function hook_commerce_adyen_pro_redirect_form_data_alter(&$data, $order) {
  // No example.
}
