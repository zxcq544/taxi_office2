<?php
function product_admin_products()
{

    // Формируем сортируемую шапку для таблицы.
    $header = array(
        'title' => array('data' => t('Title'), 'field' => 'p.title'),
        'author' => t('Author'),
        'status' => array('data' => t('Status'), 'field' => 'p.status'),
        'changed' => array('data' => t('Updated'), 'field' => 'p.changed', 'sort' => 'desc'),
        'operations' => array('data' => t('Operations')),
    );

    $query = db_select('product', 'p')->extend('PagerDefault')->extend('TableSort');
    $product_ids = $query->fields('p', array('id'))
        ->limit(50)
        ->orderByHeader($header)
        ->execute()
        ->fetchCol();

    $products = product_load_multiple($product_ids);

    // Подготавливаем список продуктов.
    $destination = drupal_get_destination();
    $rows = array();
    foreach ($products as $product) {
        $rows[$product->id] = array(
            'title' => array(
                'data' => array(
                    '#type' => 'link',
                    '#title' => $product->title,
                    '#href' => 'product/' . $product->id,
                ),
            ),
            'author' => theme('username', array('account' => $product)),
            'status' => product_status_get_title($product->status),
            'changed' => format_date($product->changed, 'short'),
        );

        // Формируем список доступных операций над текущим продуктом.
        $operations = array();
        if (product_access('update', 'product', $product)) {
            $operations['edit'] = array(
                'title' => t('edit'),
                'href' => 'product/' . $product->id . '/edit',
                'query' => $destination,
            );
        }
        if (product_access('delete', 'product', $product)) {
            $operations['delete'] = array(
                'title' => t('delete'),
                'href' => 'product/' . $product->id . '/delete',
                'query' => $destination,
            );
        }

        $rows[$product->id]['operations'] = array();
        if (count($operations) > 1) {
            // Выводим операции в виде списка.
            $rows[$product->id]['operations'] = array(
                'data' => array(
                    '#theme' => 'links__product_operations',
                    '#links' => $operations,
                    '#attributes' => array('class' => array('links', 'inline')),
                ),
            );
        } elseif (!empty($operations)) {
            // Выводим первую и единственную операцию.
            $link = reset($operations);
            $rows[$product->id]['operations'] = array(
                'data' => array(
                    '#type' => 'link',
                    '#title' => $link['title'],
                    '#href' => $link['href'],
                    '#options' => array('query' => $link['query']),
                ),
            );
        }
    }

    $page = array();
    $page['products'] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $rows,
        '#empty' => t('No products available.'),
    );

    $page['pager'] = array('#markup' => theme('pager'));
    return $page;
}