#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_ORDERS 16
#define MAX_DRIVERS 8
#define MAX_FACTORIES 8
#define MAX_FLOWS 32
#define MAX_USERS 8

typedef enum {
  ORDER_CREATED,
  ORDER_DISPATCHED,
  ORDER_IN_TRANSIT,
  ORDER_COMPLETED,
  ORDER_CANCELLED
} OrderStatus;

typedef struct {
  int id;
  char customer[64];
  char factory[64];
  int driverId;
  double amount;
  OrderStatus status;
} Order;

typedef struct {
  int id;
  char name[64];
  char phone[24];
  bool available;
} Driver;

typedef struct {
  int id;
  char name[64];
  char type[24];
  char city[32];
  bool active;
} Factory;

typedef struct {
  int id;
  int orderId;
  char direction[16];
  double amount;
  char note[64];
} FundFlow;

typedef struct {
  char username[32];
  char password[32];
  char role[16];
} User;

typedef struct {
  Order orders[MAX_ORDERS];
  int orderCount;
  Driver drivers[MAX_DRIVERS];
  int driverCount;
  Factory factories[MAX_FACTORIES];
  int factoryCount;
  FundFlow flows[MAX_FLOWS];
  int flowCount;
  User users[MAX_USERS];
  int userCount;
} AppState;

const char *order_status_to_str(OrderStatus status) {
  switch (status) {
    case ORDER_CREATED:
      return "CREATED";
    case ORDER_DISPATCHED:
      return "DISPATCHED";
    case ORDER_IN_TRANSIT:
      return "IN_TRANSIT";
    case ORDER_COMPLETED:
      return "COMPLETED";
    case ORDER_CANCELLED:
      return "CANCELLED";
    default:
      return "UNKNOWN";
  }
}

OrderStatus parse_status(const char *status) {
  if (strcmp(status, "CREATED") == 0) return ORDER_CREATED;
  if (strcmp(status, "DISPATCHED") == 0) return ORDER_DISPATCHED;
  if (strcmp(status, "IN_TRANSIT") == 0) return ORDER_IN_TRANSIT;
  if (strcmp(status, "COMPLETED") == 0) return ORDER_COMPLETED;
  return ORDER_CANCELLED;
}

void seed_data(AppState *app) {
  app->orders[0] = (Order){1001, "华北养殖场", "丰收饲料厂", 1, 23800.0, ORDER_CREATED};
  app->orders[1] = (Order){1002, "顺兴食品", "金穗面粉厂", 2, 12500.0, ORDER_IN_TRANSIT};
  app->orderCount = 2;

  app->drivers[0] = (Driver){1, "王师傅", "13800000001", true};
  app->drivers[1] = (Driver){2, "李师傅", "13800000002", false};
  app->driverCount = 2;

  app->factories[0] = (Factory){11, "金穗面粉厂", "FLOUR", "郑州", true};
  app->factories[1] = (Factory){12, "丰收饲料厂", "FEED", "新乡", true};
  app->factoryCount = 2;

  app->flows[0] = (FundFlow){5001, 1001, "INCOME", 23800.0, "订单回款"};
  app->flows[1] = (FundFlow){5002, 1002, "EXPENSE", 3800.0, "司机运费"};
  app->flowCount = 2;

  app->users[0] = (User){"admin", "admin123", "ADMIN"};
  app->users[1] = (User){"finance", "finance123", "FINANCE"};
  app->users[2] = (User){"ops", "ops123", "OPS"};
  app->userCount = 3;
}

bool login(const AppState *app, const char *username, const char *password, User *result) {
  for (int i = 0; i < app->userCount; i++) {
    if (strcmp(app->users[i].username, username) == 0 && strcmp(app->users[i].password, password) == 0) {
      *result = app->users[i];
      return true;
    }
  }
  return false;
}

Order *find_order(AppState *app, int orderId) {
  for (int i = 0; i < app->orderCount; i++) {
    if (app->orders[i].id == orderId) return &app->orders[i];
  }
  return NULL;
}

void print_orders(const AppState *app) {
  puts("GET /api/orders");
  for (int i = 0; i < app->orderCount; i++) {
    const Order *o = &app->orders[i];
    printf("{\"id\":%d,\"customer\":\"%s\",\"factory\":\"%s\",\"driverId\":%d,\"amount\":%.2f,\"status\":\"%s\"}\n",
           o->id, o->customer, o->factory, o->driverId, o->amount, order_status_to_str(o->status));
  }
}

void update_order_status(AppState *app, int orderId, const char *status) {
  puts("PATCH /api/orders/:id/status");
  Order *order = find_order(app, orderId);
  if (order == NULL) {
    printf("{\"error\":\"order %d not found\"}\n", orderId);
    return;
  }

  order->status = parse_status(status);
  printf("{\"ok\":true,\"id\":%d,\"status\":\"%s\"}\n", order->id, order_status_to_str(order->status));
}

void print_drivers(const AppState *app) {
  puts("GET /api/drivers");
  for (int i = 0; i < app->driverCount; i++) {
    const Driver *d = &app->drivers[i];
    printf("{\"id\":%d,\"name\":\"%s\",\"phone\":\"%s\",\"available\":%s}\n",
           d->id, d->name, d->phone, d->available ? "true" : "false");
  }
}

void print_factories(const AppState *app) {
  puts("GET /api/factories");
  for (int i = 0; i < app->factoryCount; i++) {
    const Factory *f = &app->factories[i];
    printf("{\"id\":%d,\"name\":\"%s\",\"type\":\"%s\",\"city\":\"%s\",\"active\":%s}\n",
           f->id, f->name, f->type, f->city, f->active ? "true" : "false");
  }
}

void print_fund_flows(const AppState *app) {
  puts("GET /api/funds/flows");
  for (int i = 0; i < app->flowCount; i++) {
    const FundFlow *f = &app->flows[i];
    printf("{\"id\":%d,\"orderId\":%d,\"direction\":\"%s\",\"amount\":%.2f,\"note\":\"%s\"}\n",
           f->id, f->orderId, f->direction, f->amount, f->note);
  }
}

int main(void) {
  AppState app = {0};
  seed_data(&app);

  User current = {0};
  if (!login(&app, "admin", "admin123", &current)) {
    puts("{\"error\":\"unauthorized\"}");
    return 1;
  }

  printf("POST /api/login => {\"user\":\"%s\",\"role\":\"%s\"}\n", current.username, current.role);
  print_orders(&app);
  update_order_status(&app, 1001, "DISPATCHED");
  print_drivers(&app);
  print_factories(&app);
  print_fund_flows(&app);

  return 0;
}
