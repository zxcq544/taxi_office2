<?php
function product_page_title($product)
{
    return $product->title;
}

function product_page_view($product)
{
    // For markup consistency with other pages, use product_view_multiple() rather than product_view().
    $products = product_view_multiple(array($product->id => $product), 'full');

    return $products;
}

function product_page_add()
{
    global $user;
    $name = variable_get('anonymous', t('Anonymous'));
    if ($user->uid && $account = user_load($user->uid)) {
        $name = $account->name;
    }

    $product = entity_create('product', array('uid' => $user->uid, 'name' => $name));
    return drupal_get_form('product_form', $product, 'create');
}

function product_page_edit($product)
{
    drupal_set_title(t('<em>Edit</em> @title', array('@title' => $product->title)), PASS_THROUGH);
    return drupal_get_form('product_form', $product);
}

function product_form($form, &$form_state, $product, $op = 'update')
{
    $form['#attributes']['class'][] = 'product-form';

    $form['product'] = array(
        '#type' => 'value',
        '#value' => $product,
    );

    $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Title'),
        '#default_value' => !empty($product->title) ? $product->title : '',
        '#required' => TRUE,
    );

    $form['additional_settings'] = array(
        '#type' => 'vertical_tabs',
        '#weight' => 99,
    );

    // Информация об авторе продукта.
    $form['author'] = array(
        '#type' => 'fieldset',
        '#access' => user_access('administer products'),
        '#title' => t('Authoring information'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#group' => 'additional_settings',
        '#attributes' => array(
            'class' => array('product-form-author'),
        ),
        '#attached' => array(
            'js' => array(
                drupal_get_path('module', 'product') . '/js/product-fieldset-summaries.js',
                array(
                    'type' => 'setting',
                    'data' => array('anonymous' => variable_get('anonymous', t('Anonymous'))),
                ),
            ),
        ),
        '#weight' => 90,
    );
    $form['author']['name'] = array(
        '#type' => 'textfield',
        '#title' => t('Authored by'),
        '#maxlength' => 60,
        '#autocomplete_path' => 'user/autocomplete',
        '#default_value' => !empty($product->name) ? $product->name : '',
        '#weight' => -1,
        '#description' => t('Leave blank for %anonymous.', array('%anonymous' => variable_get('anonymous', t('Anonymous')))),
    );
    $form['author']['date'] = array(
        '#type' => 'textfield',
        '#title' => t('Authored on'),
        '#maxlength' => 25,
        '#description' => t('Format: %time. The date format is YYYY-MM-DD and %timezone is the time zone offset from UTC. Leave blank to use the time of form submission.', array('%time' => !empty($product->created) ? format_date($product->created, 'custom', 'Y-m-d H:i:s O') : format_date(REQUEST_TIME, 'custom', 'Y-m-d H:i:s O'), '%timezone' => !empty($product->created) ? format_date($product->created, 'custom', 'O') : format_date(REQUEST_TIME, 'custom', 'O'))),
        '#default_value' => !empty($product->created) ? format_date($product->created, 'custom', 'Y-m-d H:i:s O') : '',
    );

    // Статусы продукта.
    $form['options'] = array(
        '#type' => 'fieldset',
        '#access' => user_access('administer products'),
        '#title' => t('Product status'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#group' => 'additional_settings',
        '#attributes' => array(
            'class' => array('product-form-options'),
        ),
        '#attached' => array(
            drupal_get_path('module', 'product') . '/js/product-fieldset-summaries.js',
        ),
        '#weight' => 95,
    );
    $form['options']['status'] = array(
        '#type' => 'radios',
        '#title' => t('Status'),
        '#default_value' => isset($product->status) ? $product->status : PRODUCT_PENDING,
        '#options' => array(
            PRODUCT_PENDING => product_status_get_title(PRODUCT_PENDING),
            PRODUCT_COMPLETED => product_status_get_title(PRODUCT_COMPLETED),
            PRODUCT_CANCELED => product_status_get_title(PRODUCT_CANCELED),
        ),
    );

    // Кнопка отправки формы.
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#weight' => 5,
    );

    // Если пользователь редактирует продукт и у него есть разрешение для удаления продукта,
    // то добавляем кнопку "Удалить".
    if ($op == 'update' && product_access('delete', 'product', $product)) {
        $form['actions']['delete'] = array(
            '#type' => 'submit',
            '#value' => t('Delete'),
            '#weight' => 15,
            '#submit' => array('product_form_delete_submit'),
        );
    }

    // Прикрепляем все созданные поля сущности к форме.
    field_attach_form('product', $product, $form, $form_state);

    return $form;
}


function product_form_validate($form, &$form_state)
{
    $product = $form_state['values']['product'];

    // Проверяем корректность автора.
    $product->uid = 0;
    if (!empty($form_state['values']['name'])) {
        if ($account = user_load_by_name($form_state['values']['name'])) {
            $product->uid = $account->uid;
        } elseif ($form_state['values']['name'] != variable_get('anonymous', t('Anonymous'))) {
            form_set_error('name', t('The username %name does not exist.', array('%name' => $form_state['values']['name'])));
        }
    }

    // Проверяем корректность даты публикации.
    if (!empty($form_state['values']['date'])) {
        if (strtotime($form_state['values']['date']) === FALSE) {
            form_set_error('date', t('You have to specify a valid date.'));
        } else {
            $date = date_create($form_state['values']['date']);
            $product->created = $date->getTimestamp();
        }
    } else {
        $product->created = REQUEST_TIME;
    }
}


function product_form_submit($form, &$form_state)
{
    $product = $form_state['values']['product'];
    $product->title = $form_state['values']['title'];
    $product->changed = REQUEST_TIME;
    $product->status = $form_state['values']['status'];

    field_attach_submit('product', $product, $form, $form_state);

    $is_new = !empty($product->is_new) ? TRUE : FALSE;
    $product->save();

    if ($is_new) {
        drupal_set_message(t('%title has been created.', array('%title' => $product->title)));
    } else {
        drupal_set_message(t('%title has been updated.', array('%title' => $product->title)));
    }

    $form_state['redirect'] = array('product/' . $product->id);
}

function product_form_delete_submit($form, &$form_state)
{
    $destination = array();
    if (isset($_GET['destination'])) {
        $destination = drupal_get_destination();
        unset($_GET['destination']);
    }
    $product = $form_state['values']['product'];
    $form_state['redirect'] = array('product/' . $product->id . '/delete', array('query' => $destination));
}

function product_delete_confirm($form, &$form_state, $product)
{
    $form['product_id'] = array('#type' => 'value', '#value' => $product->id);
    return confirm_form($form,
        t('Are you sure you want to delete %title?', array('%title' => $product->title)),
        'products',
        t('This action cannot be undone.'),
        t('Delete'),
        t('Cancel')
    );
}

function product_delete_confirm_submit($form, &$form_state)
{
    if ($form_state['values']['confirm']) {
        $product = product_load($form_state['values']['product_id']);
        product_delete($form_state['values']['product_id']);
        drupal_set_message(t('%title has been deleted.', array('%title' => $product->title)));
    }

    $form_state['redirect'] = 'admin/config/product/products';
}