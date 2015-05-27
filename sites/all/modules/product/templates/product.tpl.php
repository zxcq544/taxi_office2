<div id="product-<?php print $product->id; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>


    <?php $current_page_is_products = preg_match("/products/", $_SERVER["REQUEST_URI"]);
    $a=2;
    ?>
    <?php if ($current_page_is_products){ ?>
        <a href="/product/<?= $product->id ?>"><?= $product->title ?></a>
    <?php }else{ ?>
        <?= $product->title ?>
    <?php } ?>

    <div class="submitted">
        <?php print $submitted; ?>
    </div>

    <div class="content"<?php print $content_attributes; ?>>
        <?php print render($content); ?>
    </div>

</div>