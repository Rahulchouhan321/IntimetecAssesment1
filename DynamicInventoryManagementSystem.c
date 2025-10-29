#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN     50
#define MAX_PRICE        100000.0
#define MAX_PRODUCT_ID   10000
#define MAX_PRODUCTS     100
#define MAX_QUANTITY     1000000
#define MIN_PRICE        0.0
#define MIN_PRODUCT_ID   1
#define MIN_PRODUCTS     1
#define MIN_QUANTITY     0

typedef struct Product 
{
    int id;
    char name[MAX_NAME_LEN];
    float price;
    int quantity;
} Product;

char *my_strstr(const char *string, const char *substring);

int   inputProductID();
void  inputProductName(char *productName);
float inputProductPrice();
int   inputProductQuantity();
void  inputPriceRange(float *lowerPrice, float *higherPrice);
void  inputProductDetails(Product *productsPtr);

void  addProduct(Product **products, int *productsDataSize);
void  deleteByID(Product **products, int *productsDataSize);
void  searchByID(Product *products, int productsDataSize);
void  searchByName(Product *products, int productsDataSize);
void  searchByPriceRange(Product *products, int productsDataSize);
void  updateQuantity(Product *products, int productsDataSize);
void  viewProducts(Product *products, int productsDataSize);


int main(void) 
{  
    int initialNumberOfProducts;
    Product *products = NULL;

    do
    {
        printf("Enter initial number of products: ");

        if (scanf("%d", &initialNumberOfProducts) != 1)
        {
            printf("Invalid input! Please enter a numeric value.\n");
            while (getchar() != '\n');
            initialNumberOfProducts = -1;
            continue;
        }

        if (initialNumberOfProducts < MIN_PRODUCTS || initialNumberOfProducts > MAX_PRODUCTS)
        {
            printf("Invalid number of products! Must be between %d and %d.\n",
                                                  MIN_PRODUCTS, MAX_PRODUCTS);
        }

    } while (initialNumberOfProducts < MIN_PRODUCTS || initialNumberOfProducts > MAX_PRODUCTS);


    products = (Product *)calloc(initialNumberOfProducts, sizeof(Product));

    for (int i = 0; i < initialNumberOfProducts; i++) 
    {
        printf("\nEnter details for product %d:\n", i + 1);
        inputProductDetails(&products[i]);
    }

    int menuChoice;
    while (1) 
    {
        printf("\n========== Inventory Menu ==========\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity of a Product\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Products by Name\n");
        printf("6. Search Products by Price Range\n");
        printf("7. Delete Product by ID\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &menuChoice);

        switch (menuChoice) 
        {
            case 1: addProduct(&products, &initialNumberOfProducts); break;
            case 2: viewProducts(products, initialNumberOfProducts); break;
            case 3: updateQuantity(products, initialNumberOfProducts); break;
            case 4: searchByID(products, initialNumberOfProducts); break;
            case 5: searchByName(products, initialNumberOfProducts); break;
            case 6: searchByPriceRange(products, initialNumberOfProducts); break;
            case 7: deleteByID(&products, &initialNumberOfProducts); break;
            case 8:
                free(products);
                printf("Memory freed. Exiting program...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

char *my_strstr(const char *string, const char *substring) 
{
    if (!*substring)
        return (char *)string;

    const char *stringPtr;
    const char *substringPtr;

    while (*string) 
    {
        stringPtr = string;
        substringPtr = substring;

        while (*stringPtr && *substringPtr && (*stringPtr == *substringPtr)) 
        {
            stringPtr++;
            substringPtr++;
        }

        if (!*substringPtr)
            return (char *)string;

        string++;
    }

    return NULL;
}
int inputProductID() 
{
    int productId;

    while (1) 
    {
        printf("Product ID (%d -- %d): ", MIN_PRODUCT_ID, MAX_PRODUCT_ID);

        if (scanf("%d", &productId) != 1) 
        {
            printf("Invalid input! Please enter a numeric value.\n");
            while (getchar() != '\n');  
            continue;
        }

        if (productId < MIN_PRODUCT_ID || productId > MAX_PRODUCT_ID) 
        {
            printf("Invalid Product ID! Must be between %d and %d.\n", 
                                        MIN_PRODUCT_ID, MAX_PRODUCT_ID);
            continue;
        }

        break;
    }

    return productId;
}

void inputProductName(char *productName) 
{
    int validInput = 0;
    getchar();
    while (!validInput) 
    {
        printf("Product Name (1 -- %d chars): ", MAX_NAME_LEN - 1);
        scanf("%[^\n]%*c", productName);
        int productNameLen = strlen(productName);
        if (productNameLen < 1)
            printf("Name cannot be empty! Try again.\n");
        else if (productNameLen >= MAX_NAME_LEN)
            printf("Name too long (max %d chars)! Try again.\n", MAX_NAME_LEN - 1);
        else
            validInput = 1;
    }
}

float inputProductPrice() 
{
    float productPrice;

    while (1) 
    {
        printf("Price (%.2f -- %.2f): ", MIN_PRICE, MAX_PRICE);

        if (scanf("%f", &productPrice) != 1) 
        {
            printf("Invalid input! Please enter a numeric value.\n");
            while (getchar() != '\n');
            continue;
        }

        if (productPrice < MIN_PRICE || productPrice > MAX_PRICE)
        {
            printf("Invalid Price! Must be between %.2f and %.2f.\n",
                   MIN_PRICE, MAX_PRICE);
            continue;
        }

        break;
    }

    return productPrice;
}

int inputProductQuantity() 
{
    int productQuantity;

    while (1) 
    {
        printf("Quantity (%d -- %d): ", MIN_QUANTITY, MAX_QUANTITY);

        if (scanf("%d", &productQuantity) != 1) 
        {
            printf("Invalid input! Please enter a numeric value.\n");
            while (getchar() != '\n');
            continue;
        }

        if (productQuantity < MIN_QUANTITY || productQuantity > MAX_QUANTITY)
        {
            printf("Invalid Quantity! Must be between %d and %d.\n",
                   MIN_QUANTITY, MAX_QUANTITY);
            continue;
        }

        break;
    }

    return productQuantity;
}

void inputPriceRange(float *lowerPrice, float *higherPrice) 
{
    while (1) 
    {
        printf("Enter lower and upper price range: ");

        if (scanf("%f %f", lowerPrice, higherPrice) != 2) 
        {
            printf("Invalid input! Please enter two numeric values.\n");
            while (getchar() != '\n');
            continue;
        }

        if (*lowerPrice > *higherPrice || *lowerPrice < MIN_PRICE || *higherPrice > MAX_PRICE)
        {
            printf("Invalid range! Lower must be ≤ upper and both between %.2f and %.2f.\n",
                   MIN_PRICE, MAX_PRICE);
            continue;
        }

        break;
    }
}

void inputProductDetails(Product *productPtr) 
{
    productPtr->id = inputProductID();
    inputProductName(productPtr->name);
    productPtr->price = inputProductPrice();
    productPtr->quantity = inputProductQuantity();
}

void addProduct(Product **products, int *productsDataSize) 
{
     Product *tempPtr = realloc(*products, (*productsDataSize + 1) * sizeof(Product));
    
    if (tempPtr == NULL) 
    {
        printf("Memory allocation failed! Product not added.\n");
        return;
    }
    *products=tempPtr;
    printf("\nEnter details for new product:\n");
    inputProductDetails(&(*products)[*productsDataSize]);
    (*productsDataSize)++;
    printf("Product added successfully!\n");
}

void viewProducts(Product *products, int productsDataSize) 
{
    if (productsDataSize == 0) 
    {
        printf("No products available.\n");
        return;
    }

    printf("\n%-10s %-25s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < productsDataSize; i++) 
    {
        printf("%-10d %-25s %-10.2f %-10d\n", products[i].id
                                            , products[i].name
                                            , products[i].price
                                            , products[i].quantity);
    }
}

void updateQuantity(Product *products, int productsDataSize) 
{
    int productId, productQuantity, productFoundChecker= 0;
    printf("Enter product ID to update quantity: ");
    scanf("%d", &productId);

    for (int i = 0; i < productsDataSize; i++) 
    {
        if (products[i].id == productId) 
        {
            productQuantity = inputProductQuantity();
            products[i].quantity = productQuantity;
            printf("Quantity updated successfully!\n");
            productFoundChecker = 1;
            break;
        }
    }
    if (!productFoundChecker) printf("Product not found.\n");
}

void searchByID(Product *products, int productsDataSize) 
{
    int productId, productFoundChecker = 0;
    printf("Enter product ID to search: ");
    scanf("%d", &productId);
    printf("\n%-10s %-25s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < productsDataSize; i++) 
    {
        if (products[i].id == productId) 
        {
            printf("\nProduct found:\n");
            printf("%-10d %-25s %-10.2f %-10d\n", products[i].id
                                                , products[i].name
                                                , products[i].price
                                                , products[i].quantity);

            productFoundChecker = 1;
            
        }
    }
    if (!productFoundChecker) printf("Product not found.\n");
}

void searchByName(Product *products, int productsDataSize) 
{
    char productName[50];
    int productFoundChecker = 0;
    getchar();
    printf("Enter name or part of name to search: ");
    fgets(productName, 50, stdin);
    productName[strcspn(productName, "\n")] = '\0';
    printf("\n%-10s %-25s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < productsDataSize; i++) 
    {
        if (my_strstr(products[i].name, productName)) 
        {

    printf("%-10d %-25s %-10.2f %-10d\n", products[i].id
                                        , products[i].name
                                        , products[i].price
                                        , products[i].quantity);
            productFoundChecker = 1;
        }
    }
    if (!productFoundChecker) printf("No matching products found.\n");
}

void searchByPriceRange(Product *products, int productsDataSize) 
{
    float lowerPrice, higherPrice;
    int productFoundChecker = 0;
    inputPriceRange(&lowerPrice, &higherPrice);
    printf("\n%-10s %-25s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < productsDataSize; i++) 
    {
        if (products[i].price >= lowerPrice && products[i].price <= higherPrice) 
        {

    printf("%-10d %-25s %-10.2f %-10d\n", products[i].id
                                        , products[i].name
                                        , products[i].price
                                        , products[i].quantity);
            productFoundChecker = 1;
        }
    }
    if (!productFoundChecker) printf("No products found in this price range.\n");
}

void deleteByID(Product **products, int *productsDataSize) 
{
    int productId, productFoundChecker = 0;
    printf("Enter product ID to delete: ");
    scanf("%d", &productId);

    for (int i = 0; i < *productsDataSize; i++) 
    {
        if ((*products)[i].id == productId) 
        {
            for (int j = i; j < *productsDataSize - 1; j++) 
            {
                (*products)[j] = (*products)[j + 1];
            }
            (*productsDataSize)--;
             Product *tempPtr = realloc(*products, (*productsDataSize + 1) * sizeof(Product));
    
            if (tempPtr == NULL) 
            {
                printf("Memory allocation failed! Product not added.\n");
                return;
            }
            *products=tempPtr;
            printf("Product deleted successfully!\n");
            productFoundChecker = 1;
            break;
        }
    }
    if (!productFoundChecker) printf("Product not found.\n");
}
